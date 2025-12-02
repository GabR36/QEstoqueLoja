#include "eventocienciaop.h"
#include <QDateTime>
#include "../configuracao.h"
#include <QFile>
#include <QDomDocument>
#define ID_LOTE 1

EventoCienciaOP::EventoCienciaOP(QObject *parent, QString chnfe)
    : QObject{parent}
{
    empresaValues = Configuracao::get_All_Empresa_Values();
    fiscalValues = Configuracao::get_All_Fiscal_Values();
    chnfe_global = chnfe;

    acbr = AcbrManager::instance()->nfe();
}

void EventoCienciaOP::preencherEvento(){
    std::string dataHora = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm").toStdString();
    QString cnpjemit = empresaValues.value("cnpj_empresa");

    ini << "[EVENTO]\n";
    ini << "idLote=" << ID_LOTE << "\n";
    ini << "[EVENTO001]\n";
    ini << "cOrgao=" << "91" << "\n";
    ini << "CNPJ=" << cnpjemit.toStdString() << "\n";
    ini << "chNFe=" << chnfe_global.toStdString() << "\n";
    ini << "dhEvento=" << dataHora << "\n";
    ini << "tpEvento=" << "210210" << "\n"; //codigo ciencia da operação
    ini << "nSeqEvento=" << "1" << "\n";
    ini << "versaoEvento=" << "1.00" << "\n";
    // ini << "nProt=" << nprot.toStdString() << "\n";
    // ini << "xJust=" << "todos produtos do pedido devolvidos" << "\n";

}

QString EventoCienciaOP::getCampo(const QString &texto, const QString &campo)
{
    QRegularExpression re(campo + R"(=([^\r\n]*))");
    QRegularExpressionMatch m = re.match(texto);
    return m.hasMatch() ? m.captured(1).trimmed() : "";
}

QString EventoCienciaOP::gerarEnviar(){
    preencherEvento();



    try {
        acbr->LimparListaEventos();
        acbr->CarregarEventoINI(ini.str());
        acbr->Assinar();
            // nfe->GravarXml(0, "xml_naoaut_nota_"+ nnf + ".xml", "./xml");
        acbr->Validar();


        std::string retorno = acbr->EnviarEvento(1);
        QString ret = QString::fromUtf8(retorno.c_str());
        qDebug() << "Retorno SEFAZ Evento:" << ret;
        return ret;
        //nfce->Imprimir("", 1, "", true, std::nullopt, std::nullopt, std::nullopt);

    }
    catch (std::exception &e) {
        qDebug() << "Erro std::exception:" << e.what();
        return QString("Erro ao enviar evento cancel:\n%1").arg(e.what());
    }
    catch (...) {
        qDebug() << "Erro desconhecido!";
        return QString("Erro desconhecido ao enviar evento cancel ");
    }

}

EventoRetornoInfo EventoCienciaOP::gerarEnviarRetorno()
{
    preencherEvento();
    EventoRetornoInfo info;

    try {
        acbr->LimparListaEventos();
        acbr->CarregarEventoINI(ini.str());
        acbr->Assinar();
        acbr->Validar();

        std::string retorno = acbr->EnviarEvento(1);
        QString ret = QString::fromUtf8(retorno.c_str());
        // QFile file("retorno_evento.txt");
        // QString ret;

        // if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //     ret = QString::fromUtf8(file.readAll());
        //     file.close();
        // } else {
        //     qDebug() << "Erro ao abrir retorno.txt";
        // }

        qDebug() << "Retorno SEFAZ Evento:" << ret;

        // 1. obtém caminho do evento gravado
        QString eventoPath = getCampo(ret, "arquivo");
        info.xmlPath = eventoPath;


        // 2. lê e parseia o XML completo
        QFile f(eventoPath);
        if (f.open(QIODevice::ReadOnly)) {
            QDomDocument doc;
            doc.setContent(&f);
            f.close();

            auto ev = doc.firstChildElement("procEventoNFe")
                          .firstChildElement("retEvento")
                          .firstChildElement("infEvento");

            info.cStat   = ev.firstChildElement("cStat").text();
            info.xMotivo = ev.firstChildElement("xMotivo").text();
            info.nProt   = ev.firstChildElement("nProt").text();

        }

        return info;
    }
    catch (...) {
        info.cStat = "-1";
        info.xMotivo = "ERRO DESCONHECIDO";
        return info;
    }
}
