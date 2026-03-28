#include "produtovenda_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QDebug>
#include "../util/datautil.h"

ProdutoVenda_repository::ProdutoVenda_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
}

void ProdutoVenda_repository::listarProdutosVenda(QSqlQueryModel *model){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (listarProdutosVenda)";
        return;
    }

    if (!model) {
        qDebug() << "Model inválido em listarProdutosVenda";
        return;
    }

    model->setQuery("SELECT * FROM produtos_vendidos",db);
    if (model->lastError().isValid()) {
        qDebug() << "Erro SQL:" << model->lastError().text();
    }

    db.close();

}

QList<ProdutoVendidoDTO> ProdutoVenda_repository::getProdutosVendidos(qlonglong idVenda)
{
    QList<ProdutoVendidoDTO> lista;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (getProdutosVendidos)";
        return lista;
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT pv.id_produto, pv.id_venda, p.descricao, pv.quantidade, pv.preco_vendido, "
        "pv.adicionado_em, pv.atualizado_em, pv.emitido_nf "
        "FROM produtos_vendidos pv "
        "JOIN produtos p ON pv.id_produto = p.id "
        "WHERE pv.id_venda = :id_venda"
        );

    query.bindValue(":id_venda", idVenda);

    if (!query.exec()) {
        qDebug() << "Erro ao executar query:" << query.lastError().text();
        db.close();
        return lista;
    }

    while (query.next()) {
        ProdutoVendidoDTO dto;
        dto.idProduto = query.value(0).toLongLong();
        dto.idVenda = query.value(1).toLongLong();
        dto.descricao = query.value(2).toString();
        dto.quantidade = query.value(3).toDouble();
        dto.precoVendido = query.value(4).toDouble();
        dto.adicionadoEm = query.value(5).toString();
        dto.atualizadoEm  = query.value(6).toString();
        dto.emitidoNf = query.value(7).toBool();

        lista.append(dto);
    }

    db.close();
    return lista;
}

void ProdutoVenda_repository::listarProdutosVendidosFromVenda(
    qlonglong idvenda,
    QSqlQueryModel* model)
{
    if (!model) {
        qDebug() << "Model inválido em listarProdutosVendidosFromVenda";
        return;
    }

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (listarProdutosVendidosFromVenda)";
        return;
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT pv.id, p.descricao, pv.quantidade, pv.preco_vendido "
        "FROM produtos_vendidos pv "
        "JOIN produtos p ON pv.id_produto = p.id "
        "WHERE pv.id_venda = :idvenda"
        );

    query.bindValue(":idvenda", idvenda);

    if (!query.exec()) {
        qDebug() << "Erro ao executar query:" << query.lastError().text();
        db.close();
        return;
    }

    model->setQuery(query);

    if (model->lastError().isValid()) {
        qDebug() << "Erro ao setar model:" << model->lastError().text();
    }

    db.close();
}

bool ProdutoVenda_repository::deletarProdutoVendido(qlonglong id){

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (listarProdutosVendidosFromVenda)";
        return false;
    }

    QSqlQuery query(db);

    query.prepare("DELETE FROM produtos_vendidos WHERE id = :id");
    query.bindValue(":id", id);

    if(!query.exec()){
        qDebug() << "Query DeletarProdutoVendido nao rodou";
        return false;
    }else{
        qDebug() << "Produto Venda Deletado.";
        return true;
    }

}

bool ProdutoVenda_repository::deletarPorIdVenda(qlonglong idvenda){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (deletarPorIdVenda)";
        return false;
    }

    QSqlQuery query(db);

    query.prepare("DELETE FROM produtos_vendidos WHERE id_venda = :idvenda");
    query.bindValue(":idvenda", idvenda);

    if(!query.exec()){
        qDebug() << "Query não rodou deletarPorIdVenda";
        db.close();
        return false;
    }else{
        db.close();
        return true;
    }
}

int ProdutoVenda_repository::contarProdutosVendidosFromVenda(qlonglong idvenda){
    int quantidade = 0;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (contarProdutosVendidosFromVenda)";
        return -1;
    }

    QSqlQuery query(db);

    query.prepare("SELECT COUNT(*) FROM produtos_vendidos WHERE id_venda = :idvenda");
    query.bindValue(":idvenda", idvenda);

    if(!query.exec()){
        qDebug() << "Erro SQL:" << query.lastError();
        db.close();
        return -1;
    }

    if(query.next()){
        quantidade = query.value(0).toInt();
    }

    db.close();
    return quantidade;
}

ProdutoVendidoDTO ProdutoVenda_repository::getProdutoVendido(qlonglong id){
    ProdutoVendidoDTO prod;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (getProdutoVendido)";
        return prod;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id_produto, id_venda, quantidade, preco_vendido, adicionado_em, "
                  "atualizado_em, emitido_nf FROM produtos_vendidos "
                  "WHERE id = :id");
    query.bindValue(":id", id);
    if(!query.exec()){
        qDebug() << "Query getProdutoVendido nao rodou";
        db.close();
        return prod;
    }

    while (query.next()) {
        prod.idProduto = query.value("id_produto").toLongLong();
        prod.idVenda = query.value("id_venda").toLongLong();
        prod.quantidade = query.value("quantidade").toDouble();
        prod.precoVendido = query.value("preco_vendido").toDouble();
        prod.adicionadoEm = query.value("adicionado_em").toString();
        prod.atualizadoEm = query.value("atualizado_em").toString();
        prod.emitidoNf = query.value("emitido_nf").toBool();

    }
    return prod;

}

bool ProdutoVenda_repository::marcarComoEmitidoNf(qlonglong idVenda, bool emitirTodos)
{
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (marcarComoEmitidoNf)";
        return false;
    }

    QString data = DataUtil::getDataAgoraUS();
    QString sql;

    if (emitirTodos) {
        sql = "UPDATE produtos_vendidos SET emitido_nf = 1, atualizado_em = :data "
              "WHERE id_venda = :idvenda";
    } else {
        sql = "UPDATE produtos_vendidos SET emitido_nf = 1, atualizado_em = :data "
              "WHERE id_venda = :idvenda "
              "AND id_produto IN (SELECT id FROM produtos WHERE nf = 1)";
    }

    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(":data", data);
    query.bindValue(":idvenda", idVenda);

    if (!query.exec()) {
        qDebug() << "Erro marcarComoEmitidoNf:" << query.lastError().text();
        db.close();
        return false;
    }

    db.close();
    return true;
}

bool ProdutoVenda_repository::inserir(ProdutoVendidoDTO prod){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (inserir)";
        return false;
    }
    QString data = DataUtil::getDataAgoraUS();

    QSqlQuery query(db);
    query.prepare("INSERT INTO produtos_vendidos (id_produto, id_venda, quantidade, "
                  "preco_vendido, adicionado_em, atualizado_em, emitido_nf) VALUES "
                  "(:idprod, :idvenda, :quantidade, :precovendido, :adicionadoem, "
                  ":atualizadoem, :emitidonf)");

    query.bindValue(":idprod", prod.idProduto);
    query.bindValue(":idvenda", prod.idVenda);
    query.bindValue(":quantidade", prod.quantidade);
    query.bindValue(":precovendido", prod.precoVendido);
    query.bindValue(":adicionadoem", data);
    query.bindValue(":atualizadoem", data);
    query.bindValue(":emitidonf", prod.emitidoNf);

    if(!query.exec()){
        qDebug() << "Query não executou  inserir produto vendido.";
        db.close();
        return false;
    }
    db.close();
    return true;
}
