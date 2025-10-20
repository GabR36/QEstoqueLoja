#include "cancelnf.h"
#include <QSqlQuery>
#include <QDateTime>
#define ID_LOTE "1"

cancelNf::cancelNf(QObject *parent, int idnf)
    : QObject{parent}
{
    acbr = AcbrManager::instance()->nfe();
    db = QSqlDatabase::database();
    pegarDados(idnf);
    acbr->LimparListaEventos();
    //acbr->LimparLista();
}
void cancelNf::pegarDados(int idnf){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. pega dados nf.";
    }
    QSqlQuery query;
    query.prepare("SELECT cnpjemit, chnfe, nprot, cuf FROM notas_fiscais WHERE id = :valor1");
    query.bindValue(":valor1", QString::number(idnf));
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
QString cancelNf::getPath(){
    std::string raw = acbr->GetPathEvento("110111");

    // Remove caracteres nulos (caso GetPath tenha buffer fixo)
    raw.erase(std::find(raw.begin(), raw.end(), '\0'), raw.end());
    QString path = QString::fromStdString(raw).trimmed();

    // raw = cnf;
    // raw.erase(std::find(raw.begin(), raw.end(), '\0'), raw.end());
    // QString cnfString = QString::fromStdString(raw).trimmed();
    // QString ret = QString("%1/%2-nfe.xml")
    //                   .arg(path, cnfString);

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
