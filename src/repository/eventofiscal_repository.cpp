#include "eventofiscal_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>
#include "../util/datautil.h"
#include "../util/datautil.h"

EventoFiscal_repository::EventoFiscal_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
}

bool EventoFiscal_repository::inserir(EventoFiscalDTO evento){
    if(!DatabaseConnection_service::open()){
        return false;
    }
    QSqlQuery q(db);

    evento.atualizadoEm = DataUtil::getDataAgoraUS();

    q.prepare(R"(
        INSERT INTO eventos_fiscais
        (tipo_evento, id_lote, cstat, justificativa, codigo, xml_path, nprot, id_nf, atualizado_em)
        VALUES
        (:tipo, :lote, :cstat, :just, :codigo, :xml, :nprot, :idnf, :atualizado)
    )");

    q.bindValue(":tipo", evento.tipoEvento);
    q.bindValue(":lote", evento.idLote);
    q.bindValue(":cstat", evento.cstat);
    q.bindValue(":just", evento.justificativa);
    q.bindValue(":codigo", evento.codigo);
    q.bindValue(":xml", evento.xmlPath);
    q.bindValue(":nprot", evento.nProt);
    q.bindValue(":idnf", evento.idNf > 0 ? QVariant(evento.idNf) : QVariant(QMetaType(QMetaType::LongLong)));
    q.bindValue(":atualizado", evento.atualizadoEm);

    if(!q.exec()){
        qDebug() << "Query inserção evento não rodou:" << q.lastError().text();
        return false;
    }else{
        return true;
    }

}

QMap<QString, int> EventoFiscal_repository::contarPorTipo(QDateTime dtIni, QDateTime dtFim)
{
    QMap<QString, int> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. contarPorTipo (eventos)";
        return resultado;
    }

    QSqlQuery q(db);
    q.prepare(R"(
        SELECT tipo_evento, COUNT(*)
        FROM eventos_fiscais
        WHERE atualizado_em BETWEEN :ini AND :fim
        GROUP BY tipo_evento
    )");
    q.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));

    if (q.exec()) {
        while (q.next())
            resultado[q.value(0).toString()] = q.value(1).toInt();
    } else {
        qDebug() << "Erro contarPorTipo (eventos):" << q.lastError().text();
    }

    db.close();
    return resultado;
}

QList<QPair<QString, QString>> EventoFiscal_repository::buscarXmlsPorPeriodo(QDateTime dtIni, QDateTime dtFim)
{
    QList<QPair<QString, QString>> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarXmlsPorPeriodo (eventos)";
        return resultado;
    }

    QSqlQuery q(db);
    q.prepare(R"(
        SELECT tipo_evento, xml_path
        FROM eventos_fiscais
        WHERE atualizado_em BETWEEN :ini AND :fim
    )");
    q.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));

    if (q.exec()) {
        while (q.next())
            resultado.append({q.value("tipo_evento").toString(), q.value("xml_path").toString()});
    } else {
        qDebug() << "Erro buscarXmlsPorPeriodo (eventos):" << q.lastError().text();
    }

    db.close();
    return resultado;
}

void EventoFiscal_repository::listarTodos(QSqlQueryModel *model)
{
    if(!model) return;

    if(!DatabaseConnection_service::open()){
        qDebug() << "Erro ao abrir banco listarTodosEventos()";
        return;
    }

    model->setQuery(R"(
        SELECT
            id,
            tipo_evento,
            cstat,
            codigo,
            atualizado_em,
            xml_path
        FROM eventos_fiscais
        ORDER BY atualizado_em DESC
    )", db);

    if(model->lastError().isValid())
        qDebug() << "Erro SQL listarTodosEventos:" << model->lastError().text();

    db.close();
}
