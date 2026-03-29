#include "produtonota_repository.h"
#include <QSqlQuery>
#include "../infra/databaseconnection_service.h"
#include <QSqlError>
#include "../util/dbutil.h"
#include "../util/datautil.h"

ProdutoNota_repository::ProdutoNota_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();

}

void ProdutoNota_repository::listarPorNota(QSqlQueryModel *model, qlonglong idNf)
{
    if (!model) return;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco listarPorNota()";
        return;
    }

    model->setQuery(QString(R"(
        SELECT
            id,
            nitem       AS Item,
            quantidade  AS Quantidade,
            descricao   AS Descrição,
            preco       AS Preço,
            adicionado  AS Adicionado,
            status      AS Status,
            un_comercial AS Unidade,
            ncm         AS NCM,
            cfop        AS CFOP,
            csosn       AS CSOSN,
            codigo_barras AS CódigoBarras
        FROM produtos_nota
        WHERE id_nf = %1
        ORDER BY nitem
    )").arg(idNf), db);

    if (model->lastError().isValid())
        qDebug() << "Erro SQL listarPorNota:" << model->lastError().text();

    db.close();
}

bool ProdutoNota_repository::inserir(ProdutoNotaDTO produtoNota){
    if(!DatabaseConnection_service::open()){
        qDebug() << "erro ao abrir banco de dados inserir produto nota";
        return false;
    }
    QString data = DataUtil::getDataAgoraUS();
    QSqlQuery q(db);
    q.prepare(
        "INSERT INTO produtos_nota "
        "(id_nf, nitem, quantidade, descricao, preco, codigo_barras, un_comercial, "
        " ncm, csosn, pis, cfop, aliquota_imposto, cst_icms, tem_st, status, adicionado, "
        "adicionado_em, atualizado_em ) "
        "VALUES "
        "(:id_nf, :nitem, :quant, :desc, :preco, :cod_barras, :un_comercial, "
        " :ncm, :csosn, :pis, :cfop, :aliquota, :cst_icms, :tem_st, :status, :adicionado, "
        ":adicionadoem, :atualizadoem)"
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
    q.bindValue(":adicionado_em", data);
    q.bindValue(":atualizado_em", data);


    if (!q.exec()) {
        qDebug() << "Erro ao inserir produto:" << q.lastError().text();
        return false;
    }
    db.close();

    return true;

}

ProdutoNotaDTO ProdutoNota_repository::getProdutoNota(qlonglong id){
    ProdutoNotaDTO prod;
    if(!DatabaseConnection_service::open()){
        qDebug() << "erro ao abrir banco de dados getProdutoNota";
        return prod;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, quantidade, descricao, preco, codigo_barras, un_comercial, "
                  "ncm, csosn, pis, cfop, aliquota_imposto, nitem, id_nf, status, cst_icms, "
                  "tem_st, id_nfDevol, adicionado, adicionado_em, atualizado_em "
                  "FROM produtos_nota WHERE id = :id");
    query.bindValue(":id", id);

    if(!query.exec()){
        qDebug() << "Query nao rodou getProdutoNota";
        db.close();
        return prod;
    }

    while(query.next()){
        prod.id = query.value("id").toLongLong();
        prod.quantidade = query.value("quantidade").toDouble();
        prod.descricao = query.value("descricao").toString();
        prod.preco = query.value("preco").toDouble();
        prod.codigoBarras = query.value("codigo_barras").toString();
        prod.uCom = query.value("un_comercial").toString();
        prod.temSt = query.value("tem_st").toBool();
        prod.ncm = query.value("ncm").toString();
        prod.csosn = query.value("csosn").toString();
        prod.pis = query.value("pis").toString();
        prod.cfop = query.value("cfop").toString();
        prod.aliquotaIcms = query.value("aliquota_imposto").toDouble();
        prod.nitem = query.value("nitem").toInt();
        prod.idNf = query.value("id_nf").toLongLong();
        prod.status = query.value("status").toString();
        prod.cstIcms = query.value("cst_icms").toString();
        prod.idNfDevol = query.value("id_nfDevol").toLongLong();
        prod.adicionado = query.value("adicionado").toBool();
        prod.adicionadoEm = query.value("adicionado_em").toString();
        prod.atualizadoEm = query.value("atualizado_em").toString();

    }

    db.close();
    return prod;

}

QString ProdutoNota_repository::getXmlPathPorId(qlonglong id){
    if(!DatabaseConnection_service::open()){
        qDebug() << "erro ao abrir banco getXmlPathPorId";
        return {};
    }

    QSqlQuery query(db);
    query.prepare("SELECT xml_path FROM produtos_nota "
                  "INNER JOIN notas_fiscais ON id_nf = notas_fiscais.id "
                  "WHERE produtos_nota.id = :id");
    query.bindValue(":id", id);

    QString xmlPath;
    if(query.exec() && query.next()){
        xmlPath = query.value("xml_path").toString();
    } else {
        qDebug() << "getXmlPathPorId: query falhou" << query.lastError().text();
    }

    db.close();
    return xmlPath;
}

bool ProdutoNota_repository::marcarComoAdicionado(qlonglong id){
    if(!DatabaseConnection_service::open()){
        qDebug() << "erro ao abrir banco marcarComoAdicionado";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE produtos_nota SET adicionado = 1, atualizado_em = :atualizadoem "
                  "WHERE id = :id");
    query.bindValue(":atualizadoem", DataUtil::getDataAgoraUS());
    query.bindValue(":id", id);

    if(!query.exec()){
        qDebug() << "marcarComoAdicionado: query falhou" << query.lastError().text();
        db.close();
        return false;
    }

    db.close();
    return true;
}

bool ProdutoNota_repository::marcarComoDevolvido(qlonglong id, qlonglong idNfDevol){
    if(!DatabaseConnection_service::open()){
        qDebug() << "erro ao abrir banco marcarComoDevolvido";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE produtos_nota SET status = 'DEVOLVIDO', id_nfDevol = :idnfdevol, "
                  "atualizado_em = :atualizadoem WHERE id = :id");
    query.bindValue(":idnfdevol", idNfDevol);
    query.bindValue(":atualizadoem", DataUtil::getDataAgoraUS());
    query.bindValue(":id", id);

    if(!query.exec()){
        qDebug() << "marcarComoDevolvido: query falhou" << query.lastError().text();
        db.close();
        return false;
    }

    db.close();
    return true;
}

QVariantMap ProdutoNota_repository::getProdutoNotaComXmlPath(qlonglong id){
    if(!DatabaseConnection_service::open()){
        qDebug() << "erro ao abrir banco getProdutoNotaComXmlPath";
        return {};
    }

    QSqlQuery query(db);
    query.prepare("SELECT produtos_nota.id, quantidade, descricao, preco, codigo_barras, "
                  "un_comercial, ncm, aliquota_imposto, csosn, nitem, pis, xml_path "
                  "FROM produtos_nota "
                  "INNER JOIN notas_fiscais ON produtos_nota.id_nf = notas_fiscais.id "
                  "WHERE produtos_nota.id = :id");
    query.bindValue(":id", id);

    if(!query.exec()){
        qDebug() << "getProdutoNotaComXmlPath: query falhou" << query.lastError().text();
        db.close();
        return {};
    }

    QVector<QVariantMap> registros = DBUtil::extrairResultados(query);
    db.close();
    return registros.isEmpty() ? QVariantMap{} : registros.first();
}
