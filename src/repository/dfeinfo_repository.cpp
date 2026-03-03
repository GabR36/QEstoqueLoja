#include "dfeinfo_repository.h"
#include <QSqlQuery>
#include "../infra/databaseconnection_service.h"
#include <QDateTime>

DfeInfo_repository::DfeInfo_repository(QObject *parent)
    : QObject{parent}
{
    m_db = DatabaseConnection_service::db();
}

QString DfeInfo_repository::getUltimaIdentificaçãoUsada(){
    if(!DatabaseConnection_service::open()){
        qDebug() << "Erro ao abrir banco";
        return "Erro ao abrir banco";
    }

    QSqlQuery q(m_db);
    q.exec("SELECT identificacao FROM dfe_info ORDER BY datetime(data_modificado) DESC LIMIT 1");

    QString ultimaAcao;

    if (q.next()) {
        ultimaAcao = q.value(0).toString();
    }
    m_db.close();

    return ultimaAcao;
}


QString DfeInfo_repository::getUltNsuResumo(){
    if(!DatabaseConnection_service::open()){
        qDebug() << "não abriu bd getultnsu";
    }
    QSqlQuery query(m_db);
    query.exec("SELECT ult_nsu FROM dfe_info WHERE identificacao = 'consulta_resumo'");

    QString ultnsu;
    while(query.next()){
        ultnsu = query.value(0).toString();
    }
    return ultnsu;
}

QString DfeInfo_repository::getUltNsuXml(){
    if(!DatabaseConnection_service::open()){
        qDebug() << "não abriu bd getultnsu";
    }
    QSqlQuery query(m_db);
    query.exec("SELECT ult_nsu FROM dfe_info WHERE identificacao = 'consulta_xml'");

    QString ultnsu;
    while(query.next()){
        ultnsu = query.value(0).toString();
    }
    return ultnsu;
}

QString DfeInfo_repository::getDataAgora(){

    QDateTime dataIngles = QDateTime::currentDateTime();
    QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");

    return dataFormatada;

}


bool DfeInfo_repository::salvarNovoUltNsuXml(const QString &ultnsuxml){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir DB em salvarNovoUltNsu:" << m_db.lastError().text();
        return false;
    }
    QString dataFormatada = getDataAgora();
    QSqlQuery query(m_db);


    query.prepare("UPDATE dfe_info SET ult_nsu = :nsu, data_modificado = :datamod "
                  "WHERE identificacao = 'consulta_xml'");
    query.bindValue(":nsu", ultnsuxml);
    query.bindValue(":datamod", dataFormatada);

    if(!query.exec()){
        qDebug() << "query update ultnsuxml falhou:" << query.lastError().text();
        qDebug() << "SQL:" << query.lastQuery();
        return false;
    } else {
        qDebug() << "query update ultnsuxml rodou ok";
        return true;
    }
}


bool DfeInfo_repository::salvarNovoUltNsuResumo(const QString &ultNsu){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir DB em salvarNovoUltNsu:" << m_db.lastError().text();
        return false;
    }
    QString dataFormatada = getDataAgora();

    QSqlQuery query(m_db);
    query.prepare("UPDATE dfe_info SET ult_nsu = :nsu, data_modificado = :datamod "
                  "WHERE identificacao = 'consulta_resumo'");
    query.bindValue(":nsu", ultNsu);
    query.bindValue(":datamod", dataFormatada);

    if(!query.exec()){
        qDebug() << "query update ultnsu falhou:" << query.lastError().text();
        qDebug() << "SQL:" << query.lastQuery();
        return false;
    } else {
        qDebug() << "query update ultnsu rodou ok";
        return true;
    }

}


bool DfeInfo_repository::atualizarDataNsu(TipoDfeInfo tipo){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir DB em atualizarDataNsu:" << m_db.lastError().text();
        return false;
    }
    qDebug() << "atualizando data modificado nsu";
    QString identificacao;
    if(tipo == TipoDfeInfo::ConsultaResumo ){
        identificacao = "consulta_resumo";
    }else if(tipo == TipoDfeInfo::ConsultaXml){
        identificacao = "consulta_xml";
    }

    QDateTime dataIngles = QDateTime::currentDateTime();
    QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");
    QSqlQuery query(m_db);
    query.prepare("UPDATE dfe_info SET data_modificado = :datamod "
                  "WHERE identificacao = :ide");
    query.bindValue(":datamod", dataFormatada );
    query.bindValue(":ide", identificacao);
    if(!query.exec()){
        qDebug() << "nao completou query atualizar data";
        return false;

    }else{
        return true;
    }
}


QString DfeInfo_repository::getMaxData(){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Banco não abriu em possoConsultar";
        return "-1";
    }

    QSqlQuery query(m_db);

    // Pega a MAIOR data_modificado (a mais recente), considerando xml e resumo
    if (!query.exec("SELECT MAX(data_modificado) FROM dfe_info")) {
        qDebug() << "Erro ao consultar dfe_info:" << query.lastError();
        return "-1";
    }

    if (!query.next()) {
        qDebug() << "Nenhum registro encontrado em dfe_info";
        return ""; // nunca consultou → pode consultar
    }

    QString dataModStr = query.value(0).toString();

    return dataModStr;
}



