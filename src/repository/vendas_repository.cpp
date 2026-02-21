#include "vendas_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>

Vendas_repository::Vendas_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
}


qlonglong Vendas_repository::getQuantidadeComprasCliente(qlonglong idcliente){
    if(!DatabaseConnection_service::open()){
        qDebug() << "erro ao abrir banco de dados. geQuantVendas";
        return 0;
    }

    qlonglong quantCompras = 0;
    QSqlQuery query(db);
    query.prepare("SELECT count(*) FROM vendas2 where id_cliente = :id");
    query.bindValue(":id", idcliente);

    if(!query.exec()){
        db.close();
        return 0;
    }
    if(!query.next()){
        db.close();
        return 0;
    }else{
        quantCompras = query.value(0).toLongLong();
        db.close();
        return quantCompras;
    }
}

double Vendas_repository::getValorTotalVendasPrazoCliente(qlonglong idcliente){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados getValorTotalVendasPrazoCliente";
        return 0.0;
    }

    QSqlQuery queryVendas(db);
    queryVendas.prepare(
        "SELECT SUM(valor_final) FROM vendas2 "
        "WHERE id_cliente = :id_cliente AND "
        "forma_pagamento = 'Prazo'"
        );
    queryVendas.bindValue(":id_cliente", idcliente);

    double totalVendas = 0.0;

    if (queryVendas.exec() && queryVendas.next()) {
        totalVendas = queryVendas.value(0).toDouble();
    } else {
        qDebug() << "Erro ao calcular total de vendas:" << queryVendas.lastError().text();
        db.close();
        return 0.0;
    }
    db.close();
    return totalVendas;
}
