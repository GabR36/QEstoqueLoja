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
void Vendas_repository::listarVendas(QSqlQueryModel *model)
{

    if (!model) {
        qDebug() << "Model inválido em listarVendas";
        return;
    }

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (listarVendas)";
        return;
    }

    model->setQuery(
        "SELECT id, valor_final, forma_pagamento, data_hora, "
        "cliente, esta_pago, total, desconto, taxa, "
        "valor_recebido, id_cliente, troco "
        "FROM vendas2 ORDER BY id DESC",
        db
        );

    if (model->lastError().isValid()) {
        qDebug() << "Erro SQL:" << model->lastError().text();
    }

    db.close();
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

void Vendas_repository::listarVendasDeAteFormaPagamento(
    QSqlQueryModel *model,
    const QString& de,
    const QString& ate,
    VendasUtil::VendasFormaPagamento formaPag)
{
    if (!model) {
        qDebug() << "Model inválido em listarVendasDeAteFormaPagamento";
        return;
    }

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco";
        return;
    }

    QSqlQuery query(db);

    QString sql =
        "SELECT id, valor_final, forma_pagamento, data_hora, cliente, "
        "esta_pago, total, desconto, taxa, valor_recebido, troco "
        "FROM vendas2 "
        "WHERE data_hora BETWEEN :de AND :ate ";

    // Adiciona filtro de forma de pagamento
    if (formaPag != VendasUtil::VendasFormaPagamento::Nenhuma) {
        sql += "AND forma_pagamento = :formaPag ";
    }

    sql += "ORDER BY id DESC";

    query.prepare(sql);

    query.bindValue(":de", de);
    query.bindValue(":ate", ate);

    // Converter enum para string do banco
    if (formaPag != VendasUtil::VendasFormaPagamento::Nenhuma) {

        QString formaString;

        switch (formaPag) {
        case VendasUtil::VendasFormaPagamento::Dinheiro:
            formaString = "Dinheiro";
            break;
        case VendasUtil::VendasFormaPagamento::Credito:
            formaString = "Crédito";
            break;
        case VendasUtil::VendasFormaPagamento::Debito:
            formaString = "Débito";
            break;
        case VendasUtil::VendasFormaPagamento::Pix:
            formaString = "Pix";
            break;
        case VendasUtil::VendasFormaPagamento::Prazo:
            formaString = "Prazo";
            break;
        case VendasUtil::VendasFormaPagamento::NaoSei:
            formaString = "Não Sei";
            break;
        default:
            break;
        }

        query.bindValue(":formaPag", formaString);
    }

    if (!query.exec()) {
        qDebug() << "Erro SQL:" << query.lastError().text();
        db.close();
        return;
    }

    model->setQuery(query);

    db.close();
}

ResumoVendasDTO Vendas_repository::calcularResumo(
    const QString& dataDe,
    const QString& dataAte,
    bool somentePrazo,
    qlonglong idCliente)
{
    ResumoVendasDTO resumo;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (calcularResumo)";
        return resumo;
    }

    QSqlQuery query(db);

    QString where = " WHERE data_hora BETWEEN :de AND :ate ";

    if (somentePrazo)
        where += " AND forma_pagamento = 'Prazo' ";

    if (idCliente > 0)
        where += " AND id_cliente = :idCliente ";

    // TOTAL E QUANTIDADE
    query.prepare(
        "SELECT SUM(total - desconto), COUNT(*) "
        "FROM vendas2 " + where
        );

    query.bindValue(":de", dataDe);
    query.bindValue(":ate", dataAte);

    if (idCliente > 0)
        query.bindValue(":idCliente", idCliente);

    if (query.exec() && query.next()) {
        resumo.total = query.value(0).toDouble();
        resumo.quantidade = query.value(1).toInt();
    }

    // PEGAR PORCENTAGEM DE LUCRO
    query.prepare("SELECT value FROM config WHERE key = 'porcent_lucro'");
    if (query.exec() && query.next()) {
        double porcent = query.value(0).toDouble() / 100.0;
        resumo.lucro = resumo.total * porcent / (1.0 + porcent);
    }

    db.close();

    return resumo;
}

bool Vendas_repository::deletarVenda(qlonglong id){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (deletarVenda)";
        return false;
    }

    QSqlQuery query(db);

    query.prepare("DELETE FROM vendas2 WHERE id = :id");
    query.bindValue(":id", id);
    if(!query.exec()){
        qDebug()<< "Não executou query deletarVenda";
        db.close();
        return false;
    }else{
        db.close();
        return true;
    }
}

bool Vendas_repository::updateNewTotalTrocoValorFinal(double total, double troco, double valorFinal,
                                                      qlonglong id){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (updateNewTotalTrocoValorFinal)";
        return false;
    }

    QSqlQuery query(db);

    query.prepare("UPDATE vendas2 SET total = :total, troco = :troco, valor_final = :final "
                  "WHERE id = :id");
    query.bindValue(":total", total);
    query.bindValue(":troco", troco);
    query.bindValue(":final", valorFinal);
    query.bindValue(":id", id);
    if(!query.exec()){
        qDebug() << "Query não rodou updateNewTotalTrocoValorFinal";
        db.close();
        return false;
    }
    db.close();
    return true;
}
