#include "entradasvendas_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

EntradasVendas_repository::EntradasVendas_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
}

QDateTime EntradasVendas_repository::getDataUltimoPagamentoFromCliente(qlonglong idCliente)
{
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados.";
        return QDateTime(); // retorna inválido
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT ev.data_hora "
        "FROM entradas_vendas ev "
        "JOIN vendas2 v ON ev.id_venda = v.id "
        "WHERE v.id_cliente = :id "
        "ORDER BY ev.data_hora DESC "
        "LIMIT 1"
        );

    query.bindValue(":id", idCliente);

    if (!query.exec()) {
        qDebug() << "Erro na query:" << query.lastError().text();
        return QDateTime(); // inválido
    }

    if (query.next()) {
        return query.value(0).toDateTime();
    }

    return QDateTime(); // nenhum pagamento encontrado
}

double EntradasVendas_repository::getValorUltimoPagamentoFromCliente(qlonglong idcliente){
    if(!DatabaseConnection_service::open()){
        qDebug() << "erro ao abrir banco de dados. getValorUltimoPagamento";
        return 0;
    }
    QSqlQuery query(db);

    query.prepare("SELECT ev.valor_final "
                  "FROM entradas_vendas ev "
                  "JOIN vendas2 v ON ev.id_venda = v.id "
                  "WHERE v.id_cliente = :id "
                  "ORDER BY ev.data_hora DESC "
                  "LIMIT 1;");
    query.bindValue(":id", idcliente);

    if(!query.exec()) {
        qDebug() << "Erro na query:" << query.lastError().text();
        db.close();
        return 0;
    }
    double valor;
    if(query.next()) {
        valor = query.value(0).toDouble();
    } else {
        db.close();
        return 0;
    }
    db.close();

    return valor;
}

double EntradasVendas_repository::getValorTotalEntradasFromClientes(qlonglong idcliente){
    if(!DatabaseConnection_service::open()){
        qDebug() << "Banco nao abriu, getValorTotalEntradasFromClientes";
        return 0;
    }

    QSqlQuery queryEntradas(db);
    queryEntradas.prepare(
        "SELECT SUM(ev.valor_final) FROM entradas_vendas ev "
        "JOIN vendas2 v ON ev.id_venda = v.id "
        "WHERE v.id_cliente = :id_cliente"
        );
    queryEntradas.bindValue(":id_cliente", idcliente);

    double totalEntradas = 0.0;
    if (queryEntradas.exec() && queryEntradas.next()) {
        totalEntradas = queryEntradas.value(0).toDouble();
    } else {
        qDebug() << "Erro ao calcular total de entradas:" << queryEntradas.lastError().text();
        db.close();
        return 0.0;
    }

    db.close();
    return totalEntradas;
}

QList<EntradaVendaDTO> EntradasVendas_repository::getEntradasFromVenda(qlonglong idvenda){
    QList<EntradaVendaDTO>lista;

    if(!DatabaseConnection_service::open()){
        qDebug() << "banco de dados nao abriu getEntradaFromVenda()";
        return lista;
    }

    QSqlQuery query(db);
    query.prepare("SELECT total, data_hora, forma_pagamento, valor_recebido, troco, "
                  "taxa, valor_final, desconto FROM entradas_vendas WHERE "
                  "id_venda = :idvenda");
    query.bindValue(":idvenda", idvenda);

    if(!query.exec()){
        qDebug() << "Query não executou getEntradaFromVenda()";
        db.close();
        return lista;
    }

    while(query.next()){
        EntradaVendaDTO dto;
        dto.dataHora = query.value("data_hora").toString();
        dto.desconto = query.value("desconto").toDouble();
        dto.formaPagamento = query.value("forma_pagamento").toString();
        dto.taxa = query.value("taxa").toDouble();
        dto.total = query.value("total").toDouble();
        dto.troco = query.value("troco").toDouble();
        dto.valorFinal = query.value("valor_final").toDouble();
        dto.valorRecebido = query.value("valor_recebido").toDouble();
        dto.idVenda = idvenda;
        lista.append(dto);
    }
    db.close();
    return lista;

}

bool EntradasVendas_repository::deletarPorIdVenda(qlonglong idvenda){
    if(!DatabaseConnection_service::open()){
        qDebug() << "banco de dados nao abriu deletarPorIdVenda()";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM entradas_vendas WHERE id_venda = :idvenda");
    query.bindValue(":idvenda", idvenda);

    if(!query.exec()){
        qDebug() << "query nao rodou deletarPorIdVenda";
        db.close();
        return false;
    }else{
        db.close();
        return true;
    }
}

