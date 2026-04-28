#include "eventocartacorrecao.h"
#include <QDebug>
#include "../util/datautil.h"
#include <QRegularExpression>
#include <QDir>
#include <QDomDocument>

#define ID_LOTE 1

EventoCartaCorrecao::EventoCartaCorrecao(QObject *parent, QString chnfe, int nSeq, QString correcao)
    : QObject{parent}
{

    confDTO = confServ.carregarTudo();
    chnfe_global = chnfe;
    nseq_global = nSeq;
    correcao_global = correcao;
    acbr = AcbrManager::instance()->nfe();
}


void EventoCartaCorrecao::preencherEvento(){
    QString dataHora = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm");
    std::string cnpjemit = confDTO.cnpjEmpresa.toStdString();
    std::string corgao = confDTO.cUfFiscal.toStdString();
    std::string chave = chnfe_global.toStdString();
    tpevento = "110110";
    std::string nseq = QString::number(nseq_global).toStdString();
    std::string correcao = correcao_global.toStdString();

    // Cabeçalho
    ini << "[EVENTO]\n";
    ini << "idLote=1\n\n";

    // Evento CCe
    ini << "[EVENTO001]\n";
    ini << "cOrgao=" <<  corgao<< "\n";
    ini << "CNPJ=" << cnpjemit << "\n";
    ini << "chNFe=" <<  chave << "\n";
    ini << "dhEvento=" << dataHora.toStdString() << "\n";
    ini << "tpEvento=" << tpevento << "\n"; // Carta de Correção
    ini << "nSeqEvento=" << nseq << "\n"; // se já tiver CCe, incrementar
    ini << "versaoEvento=1.00\n";
    ini << "xCorrecao=" << correcao << "\n";

}

void EventoCartaCorrecao::setRetornoForcado(QString &retorno){
    retornoForcado = retorno;
}

QString EventoCartaCorrecao::getCampo(const QString &texto, const QString &campo)
{
    QRegularExpression re(campo + R"(=([^\r\n]*))");
    QRegularExpressionMatch m = re.match(texto);
    return m.hasMatch() ? m.captured(1).trimmed() : "";
}


QString EventoCartaCorrecao::getCampoSecao(const QString &texto, const QString &secao, const QString &campo)
{
    QRegularExpression re("\\[" + secao + "\\]([\\s\\S]*?)(\\n\\[|$)");
    QRegularExpressionMatch match = re.match(texto);

    if (match.hasMatch()) {
        QString bloco = match.captured(1);
        return getCampo(bloco, campo);
    }
    return "";
}


EventoFiscalDTO EventoCartaCorrecao::gerarEnviarRetorno()
{
    preencherEvento();

    EventoFiscalDTO evento;
    evento.codigo = QString::fromStdString(tpevento);
    evento.idLote = ID_LOTE;
    evento.tipoEvento = "Carta de Correção";

    try {
        acbr->LimparListaEventos();
        acbr->CarregarEventoINI(ini.str());

        std::string retorno = "";

        if (!retornoForcado.isEmpty()) {
            retorno = retornoForcado.toStdString();
        } else {
            retorno = acbr->EnviarEvento(1);
        }

        QString ret = QString::fromUtf8(retorno.c_str());
        qDebug() << "Retorno SEFAZ Evento:" << ret;

        // =========================
        // 🔹 1. PRIORIDADE: Evento001
        // =========================
        evento.cstat = getCampoSecao(ret, "Evento001", "CStat");
        evento.justificativa = getCampoSecao(ret, "Evento001", "XMotivo");
        evento.nProt = getCampoSecao(ret, "Evento001", "nProt");

        // =========================
        // 🔹 2. FALLBACK: Evento (lote)
        // =========================
        if (evento.cstat.isEmpty())
            evento.cstat = getCampo(ret, "CStat");

        if (evento.justificativa.isEmpty())
            evento.justificativa = getCampo(ret, "XMotivo");

        // =========================
        // 🔹 3. Caminho do XML (se existir)
        // =========================
        QString eventoPath = getCampoSecao(ret, "Evento001", "arquivo");

        eventoPath.remove('\r');
        eventoPath.remove('\n');
        eventoPath.replace('\\', '/');
        eventoPath = QDir::cleanPath(eventoPath);

        evento.xmlPath = eventoPath;

        // =========================
        // 🔹 4. Se tem XML, sobrescreve com dados oficiais
        // =========================
        if (!eventoPath.isEmpty()) {
            QFile f(eventoPath);

            if (f.open(QIODevice::ReadOnly)) {
                QDomDocument doc;
                doc.setContent(&f);
                f.close();

                auto ev = doc.firstChildElement("procEventoNFe")
                              .firstChildElement("retEvento")
                              .firstChildElement("infEvento");

                if (!ev.isNull()) {
                    QString cStatXml = ev.firstChildElement("cStat").text();
                    QString xMotivoXml = ev.firstChildElement("xMotivo").text();
                    QString nProtXml = ev.firstChildElement("nProt").text();

                    if (!cStatXml.isEmpty())
                        evento.cstat = cStatXml;

                    if (!xMotivoXml.isEmpty())
                        evento.justificativa = xMotivoXml;

                    if (!nProtXml.isEmpty())
                        evento.nProt = nProtXml;
                }
            }
        }

        return evento;
    }
    catch (...) {
        evento.cstat = "-1";
        evento.justificativa = "ERRO DESCONHECIDO";
        return evento;
    }
}
