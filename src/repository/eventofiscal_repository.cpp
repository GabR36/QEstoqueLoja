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
        (tipo_evento, id_lote, cstat, justificativa, codigo, xml_path, nprot, id_nf)
        VALUES
        (:tipo, :lote, :cstat, :just, :codigo, :xml, :nprot, :idnf)
        )
    )");

    q.bindValue(":tipo", evento.tipoEvento);
    q.bindValue(":lote", evento.idLote);
    q.bindValue(":cstat", evento.cstat);
    q.bindValue(":just", evento.justificativa);
    q.bindValue(":codigo", evento.codigo);
    q.bindValue(":xml", evento.xmlPath);
    q.bindValue(":nprot", evento.nProt);
    q.bindValue(":id_nf", evento.idNf);

    if(!q.exec()){
        qDebug() << "Query inserção evento não rodou";
        return false;
    }else{
        return true;
    }

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
