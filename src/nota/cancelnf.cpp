#include "cancelnf.h"
#include <QSqlQuery>
#include <QDateTime>
#include <QRegularExpression>
#include "../util/NfUtilidades.h"

#define ID_LOTE "1"

cancelNf::cancelNf(QObject *parent, qlonglong idnf)
    : QObject{parent}
{
    acbr = AcbrManager::instance()->nfe();
    db = QSqlDatabase::database();
    pegarDados(idnf);
    acbr->LimparListaEventos();
    //acbr->LimparLista();
}
void cancelNf::pegarDados(qlonglong idnf){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. pega dados nf.";
    }
    QSqlQuery query;
    query.prepare("SELECT cnpjemit, chnfe, nprot, cuf FROM notas_fiscais WHERE id = :valor1");
    query.bindValue(":valor1", idnf);
    if (!query.exec()) {
        qDebug() << "Erro no query: ";
    }

    while (query.next()){
        cnpjemit = query.value(0).toString();
        chnfe = query.value(1).toString();
        nprot = query.value(2).toString();
        cuf = query.value(3).toString();
    }

}
void cancelNf::preencherEvento(){
    std::string dataHora = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm").toStdString();
    ini << "[EVENTO]\n";
    ini << "idLote=" << ID_LOTE << "\n";
    ini << "[EVENTO001]\n";
    ini << "cOrgao=" << cuf.toStdString() << "\n";
    ini << "CNPJ=" << cnpjemit.toStdString() << "\n";
    ini << "chNFe=" << chnfe.toStdString() << "\n";
    ini << "dhEvento=" << dataHora << "\n";
    ini << "tpEvento=" << "110111" << "\n";
    ini << "nSeqEvento=" << "1" << "\n";
    ini << "versaoEvento=" << "1.00" << "\n";
    ini << "nProt=" << nprot.toStdString() << "\n";
    ini << "xJust=" << "todos produtos do pedido devolvidos" << "\n";


}

QString cancelNf::getPath() {
    // Pega o buffer do ACBr
    std::string raw = acbr->GetPathEvento("110111");

    // Remove tudo após o primeiro \0 (terminador de string)
    auto nullPos = std::find(raw.begin(), raw.end(), '\0');
    raw.erase(nullPos, raw.end());

    // Converte para QString
    QString path = QString::fromStdString(raw).trimmed();

    // Normaliza todas as barras para Windows
    path.replace("/", "\\");

    return path;
}


QString cancelNf::gerarEnviar(){
    preencherEvento();
    try {
        acbr->CarregarEventoINI(ini.str());
        acbr->Assinar();
            // nfe->GravarXml(0, "xml_naoaut_nota_"+ nnf + ".xml", "./xml");
        acbr->Validar();


        std::string retorno = acbr->EnviarEvento(1);
        QString ret = QString::fromUtf8(retorno.c_str());
        qDebug() << "path evento:" << getPath();
        qDebug() << "Retorno SEFAZ:" << ret;
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

EventoFiscalDTO cancelNf::GerarEnviarRetorno(){
    EventoFiscalDTO evento;
    QString retorno = gerarEnviar();
    // QString tipo_evento, cstat, justificativa, codigo, xml_path, nprot;
    evento.idLote = 1;
    evento.codigo = EVENTO_CANCELAMENTO;
    // Usa QRegularExpression para extrair dados do texto
    QRegularExpression rx_idlote("idLote=(\\d+)");
    // QRegularExpression rx_tpEvento("tpEvento=(\\d+)");
    QRegularExpression rx_xEvento("xEvento=([^\n]+)");
    QRegularExpression rx_nProt("nProt=(\\d+)");
    QRegularExpression rx_xJust("xJust=([^<\n]+)");
    QRegularExpression rx_arquivo("arquivo=([^\n]+)");
    QRegularExpression rx_cstat("CStat=(\\d+)");
    QRegularExpression rx_dh("dhRegEvento=(\\d{2}/\\d{2}/\\d{4} \\d{2}:\\d{2}:\\d{2})");
    QRegularExpression rx_xMotivo("XMotivo=([^\n]+)");


    auto match = rx_idlote.match(retorno);
    if (match.hasMatch())  evento.idLote = match.captured(1).toInt();

    // match = rx_cstat.match(retorno);
    // if (match.hasMatch()) cstat = match.captured(1).trimmed();

    // match = rx_tpEvento.match(retorno);
    // if (match.hasMatch())  evento.codigo = match.captured(1).trimmed();

    match = rx_xEvento.match(retorno);
    if (match.hasMatch()) evento.tipoEvento = match.captured(1).trimmed();

    match = rx_nProt.match(retorno);
    if (match.hasMatch()) evento.nProt = match.captured(1).trimmed();

    match = rx_xJust.match(retorno);
    if (match.hasMatch()) evento.justificativa = match.captured(1).trimmed();

    match = rx_arquivo.match(retorno);
    if (match.hasMatch()) {
        evento.xmlPath = match.captured(1).trimmed();

        // Substitui todas as barras invertidas \ por /
        evento.xmlPath.replace("\\", "/");
    }


    match = rx_dh.match(retorno);

    QDateTime dataIngles;
    if (match.hasMatch()) {
        QString dataStr = match.captured(1);
        dataIngles = QDateTime::fromString(dataStr, "dd/MM/yyyy HH:mm:ss");
    } else {
        // Se não achar, usa a data atual
        dataIngles = QDateTime::currentDateTime();
    }
    evento.atualizadoEm = dataIngles.toString();
    // match = rx_xMotivo.match(retorno);
    // if (match.hasMatch()) {
    //     xMotivo = match.captured(1).trimmed();
    // }

    QRegularExpressionMatchIterator it = rx_cstat.globalMatch(retorno);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        evento.cstat = m.captured(1).trimmed(); // sobrescreve até o último
    }

    it = rx_xMotivo.globalMatch(retorno);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        evento.justificativa = m.captured(1).trimmed(); // sobrescreve até o último,
        // transforma justificativa em cstat
    }
    // Debug opcional
    qDebug() << "Evento:" << evento.tipoEvento << "| Prot:" << evento.nProt << "| CStat:" << evento.cstat;
    return evento;
}
