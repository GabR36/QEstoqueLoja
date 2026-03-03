#include "produtonota_repository.h"
#include <QSqlQuery>
#include "../infra/databaseconnection_service.h"
#include <QSqlError>

ProdutoNota_repository::ProdutoNota_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();

}

bool ProdutoNota_repository::inserir(ProdutoNotaDTO produtoNota){
    if(!DatabaseConnection_service::open()){
        qDebug() << "erro ao abrir banco de dados inserir produto nota";
        return false;
    }
    QSqlQuery q(db);
    q.prepare(
        "INSERT INTO produtos_nota "
        "(id_nf, nitem, quantidade, descricao, preco, codigo_barras, un_comercial, "
        " ncm, csosn, pis, cfop, aliquota_imposto, cst_icms, tem_st, status, adicionado) "
        "VALUES "
        "(:id_nf, :nitem, :quant, :desc, :preco, :cod_barras, :un_comercial, "
        " :ncm, :csosn, :pis, :cfop, :aliquota, :cst_icms, :tem_st, :status, :adicionado)"
        );

    q.bindValue(":id_nf",        produtoNota.idNf);
    q.bindValue(":nitem",        produtoNota.nitem);
    q.bindValue(":quant",        produtoNota.quantidade);
    q.bindValue(":desc",         produtoNota.descricao);
    q.bindValue(":preco",        produtoNota.preco);
    q.bindValue(":cod_barras",   produtoNota.codigoBarras);
    q.bindValue(":un_comercial", produtoNota.uCom);
    q.bindValue(":ncm",          produtoNota.ncm);
    q.bindValue(":csosn",        produtoNota.csosn);
    q.bindValue(":pis",          produtoNota.pis);
    q.bindValue(":cfop",         produtoNota.cfop);
    q.bindValue(":aliquota",     produtoNota.aliquotaIcms);
    q.bindValue(":cst_icms",     produtoNota.cstIcms);
    q.bindValue(":tem_st",       produtoNota.temSt ? 1 : 0);
    q.bindValue(":status",       "OK");
    q.bindValue(":adicionado", 0);

    if (!q.exec()) {
        qDebug() << "Erro ao inserir produto:" << q.lastError().text();
        return false;
    }
    db.close();

    return true;


}
