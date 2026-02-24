#include "vendas_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>

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

QSqlQueryModel* Vendas_repository::listarVendas(){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (listarCompras)";
        return nullptr;
    }

    auto *model = new QSqlQueryModel();
    model->setQuery("SELECT id, valor_final,forma_pagamento, data_hora, "
                    "cliente, esta_pago, total, desconto, taxa, "
                    "valor_recebido, id_cliente, troco FROM vendas2 ORDER BY id DESC", db);

    if (model->lastError().isValid()) {
        qDebug() << "Erro SQL:" << model->lastError().text();
    }
    db.close();
    return model;
}

QPair<QDate, QDate> Vendas_repository::getMinMaxData(){
    QPair<QDate, QDate> dataRange;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (getDataMinima)";
        return dataRange;
    }

    QSqlQuery query(db);

    query.exec("SELECT MIN(data_hora) FROM vendas2");
    if (query.next()) {
        dataRange.first = query.value(0).toDate();
    }else{
        qDebug() << "Data Minima query nao rodou";
        db.close();
        return dataRange;
    }

    // data nova
    query.exec("SELECT MAX(data_hora) FROM vendas2");
    if (query.next()) {
        dataRange.second = query.value(0).toDate();
    }else{
        qDebug() << "Data Maxima query nao rodou";
        db.close();
        return dataRange;
    }
    query.finish();
    db.close();
    return dataRange;
}

VendasDTO Vendas_repository::getVenda(qlonglong id){
    VendasDTO result;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (listarProdutosVenda)";
        return result;
    }

    QSqlQuery query(db);
    query.prepare("SELECT cliente, data_hora, total, forma_pagamento, valor_recebido, troco, "
                  "taxa, valor_final, desconto, esta_pago, id_cliente FROM vendas2 "
                  "WHERE id = :id_venda");
    query.bindValue(":id_venda", id);
    if(query.exec()){
        while (query.next()) {
            result.clienteNome = query.value("cliente").toString();
            result.dataHora = query.value("data_hora").toString();
            result.total = query.value("total").toDouble();
            result.formaPagamento = query.value("forma_pagamento").toString();
            result.valorRecebido = query.value("valor_recebido").toDouble();
            result.troco = query.value("troco").toDouble();
            result.taxa = query.value("taxa").toDouble();
            result.valorFinal = query.value("valor_final").toDouble();
            result.desconto = query.value("desconto").toDouble();
            result.estaPago = query.value("esta_pago").toBool();
            result.idCliente = query.value("id_cliente").toLongLong();

        }
    }else{
        db.close();
        qDebug() << "erro query venda2 imprimir";
    }
    return result;

}
