#include "produtovenda_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>


ProdutoVenda_repository::ProdutoVenda_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
}

QSqlQueryModel *ProdutoVenda_repository::listarProdutosVenda(){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (listarProdutosVenda)";
        return nullptr;
    }

    auto *model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM produtos_vendidos", db);

    if (model->lastError().isValid()) {
        qDebug() << "Erro SQL:" << model->lastError().text();
    }
    // db.close();
    return model;
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
        "SELECT pv.id_produto, pv.id_venda, p.descricao, pv.quantidade, pv.preco_vendido "
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

        lista.append(dto);
    }

    db.close();
    return lista;
}

QSqlQueryModel *ProdutoVenda_repository::listarProdutosVendidosFromVenda(qlonglong idvenda){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (listarProdutosVenda)";
        return nullptr;
    }

    auto *model = new QSqlQueryModel();
    QSqlQuery q(db);
    q.prepare(
        "SELECT pv.id, p.descricao, pv.quantidade, pv.preco_vendido "
        "FROM produtos_vendidos pv "
        "JOIN produtos p ON pv.id_produto = p.id "
        "WHERE pv.id_venda = :idvenda"
        );
    q.bindValue(":idvenda", idvenda);
    if (!q.exec()) {
        qDebug() << "Erro ao executar:" << q.lastError().text();
    }
    model->setQuery(q);

    if (model->lastError().isValid()) {
        qDebug() << "Erro SQL:" << model->lastError().text();
    }
    // db.close();
    return model;
}


