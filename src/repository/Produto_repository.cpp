#include "Produto_repository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "../services/Produto_service.h"
#include "../infra/databaseconnection_service.h"

Produto_Repository::Produto_Repository(QObject *parent)
: QObject{parent}
{
    db = DatabaseConnection_service::db();
}

bool Produto_Repository::codigoBarrasExiste(const QString &codigo)
{
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM produtos WHERE codigo_barras = :codigo");
    query.bindValue(":codigo", codigo);

    if (!query.exec()) {
        qDebug() << "SQL ERROR:" << query.lastError().text();
        db.close();
        return false;
    }

    query.next();
    db.close();
    return query.value(0).toInt() > 0;
}

bool Produto_Repository::inserir(const ProdutoDTO &p, QString &erroSQL)
{
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return false;
    }

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO produtos "
        "(quantidade, descricao, preco, codigo_barras, nf, un_comercial, "
        "preco_fornecedor, porcent_lucro, ncm, cest, aliquota_imposto, csosn, pis) "
        "VALUES "
        "(:quantidade, :descricao, :preco, :codigo_barras, :nf, :un_comercial, "
        ":preco_fornecedor, :porcent_lucro, :ncm, :cest, :aliquota_imposto, :csosn, :pis)"
        );

    query.bindValue(":quantidade", p.quantidade);
    query.bindValue(":descricao", p.descricao);
    query.bindValue(":preco", p.preco);
    query.bindValue(":codigo_barras", p.codigoBarras);
    query.bindValue(":nf", p.nf);
    query.bindValue(":un_comercial", p.uCom);
    query.bindValue(":preco_fornecedor", p.precoFornecedor);
    query.bindValue(":porcent_lucro", p.percentLucro);
    query.bindValue(":ncm", p.ncm);
    query.bindValue(":cest", p.cest);
    query.bindValue(":aliquota_imposto", p.aliquotaIcms);
    query.bindValue(":csosn", p.csosn);
    query.bindValue(":pis", p.pis);

    if (!query.exec()) {
        erroSQL = query.lastError().text();
        qDebug() << "[SQL ERROR]" << erroSQL;
        qDebug() << "[SQL QUERY]" << query.lastQuery();
        db.close();
        return false;
    }
    db.close();
    return true;
}

void Produto_Repository::listarProdutos(QSqlQueryModel* model)
{
    if (!model) {
        qDebug() << "Model inválido em listarProdutos";
        return;
    }
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return;
    }
    model->setQuery("SELECT * FROM produtos ORDER BY id DESC", db);

    db.close();
}

QSqlQueryModel* Produto_Repository::getProdutoPeloCodigo(const QString &codigoBarras){
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return nullptr;
    }

    auto *model = new QSqlQueryModel();
    QString sql = QString(
                      "SELECT * FROM produtos WHERE codigo_barras = '%1' ORDER BY id DESC"
                      ).arg(codigoBarras);
    model->setQuery(sql, db);

    if (model->lastError().isValid()) {
        qDebug() << "Erro SQL:" << model->lastError().text();
    }
    db.close();
    return model;
}


bool Produto_Repository::deletar(const QString &id, QString &erroSQL)
{
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM produtos WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        erroSQL = query.lastError().text();
        db.close();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        erroSQL = "Produto não encontrado para exclusão.";
        db.close();
        return false;
    }
    db.close();
    return true;
}

void Produto_Repository::pesquisar(const QStringList &palavras,
                                              const QString &textoNormalizado, QSqlQueryModel* model)
{
    if (!model) {
        qDebug() << "Model inválido em pesquisar";
        return;
    }

    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return;
    }

    QString sql = "SELECT * FROM produtos WHERE ";
    QStringList conditions;

    if (palavras.size() > 1) {
        for (int i = 0; i < palavras.size(); ++i) {
            conditions << QString("descricao LIKE :p%1").arg(i);
        }
        sql += conditions.join(" AND ");
    } else {
        sql += "descricao LIKE :busca OR codigo_barras LIKE :busca";
    }

    sql += " ORDER BY id DESC";

    QSqlQuery query(db);
    query.prepare(sql);

    if (palavras.size() > 1) {
        for (int i = 0; i < palavras.size(); ++i) {
            query.bindValue(QString(":p%1").arg(i), "%" + palavras[i] + "%");
        }
    } else {
        query.bindValue(":busca", "%" + textoNormalizado + "%");
    }

    if (!query.exec()) {
        qDebug() << "[SQL ERROR]" << query.lastError().text();
        db.close();
        return;
    }

    model->setQuery(query);
    db.close();
}

bool Produto_Repository::alterar(
    const ProdutoDTO &p,
    const QString &id,
    QString &erro
    ){
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return false;
    }
    QSqlQuery query(db);

    query.prepare(R"(
        UPDATE produtos SET
            quantidade = :quant,
            descricao = :desc,
            preco = :preco,
            codigo_barras = :barras,
            nf = :nf,
            un_comercial = :ucom,
            preco_fornecedor = :precoforn,
            porcent_lucro = :porcentlucro,
            ncm = :ncm,
            cest = :cest,
            aliquota_imposto = :aliquotaimp,
            csosn = :csosn,
            pis = :pis
        WHERE id = :id
    )");

    query.bindValue(":id", id);
    query.bindValue(":quant", p.quantidade);
    query.bindValue(":desc", Produto_Service::normalizeText(p.descricao));
    query.bindValue(":preco", p.preco);
    query.bindValue(":barras", p.codigoBarras);
    query.bindValue(":nf", p.nf);
    query.bindValue(":ucom", p.uCom);
    query.bindValue(":precoforn", p.precoFornecedor);
    query.bindValue(":porcentlucro", p.percentLucro);
    query.bindValue(":ncm", p.ncm);
    query.bindValue(":cest", p.cest);
    query.bindValue(":aliquotaimp", p.aliquotaIcms);
    query.bindValue(":csosn", p.csosn);
    query.bindValue(":pis", p.pis);

    if (!query.exec()) {
        erro = query.lastError().text();
        db.close();
        return false;
    }
    db.close();
    return true;
}


QStringList Produto_Repository::listarLocais()
{
    QStringList sugestoes;
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return sugestoes;
    }

    QSqlQuery query(db);
    if (query.exec("SELECT DISTINCT local FROM produtos WHERE local IS NOT NULL AND TRIM(local) != ''")) {
        while (query.next()) {
            sugestoes << query.value(0).toString();
        }
    } else {
        qDebug() << "Erro SQL listarLocais:" << query.lastError().text();
    }
    db.close();
    return sugestoes;
}

bool Produto_Repository::atualizarLocal(int id, const QString &local, QString &erroSQL)
{
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE produtos SET local = :local WHERE id = :id");
    query.bindValue(":local", local);
    query.bindValue(":id", id);

    if (!query.exec()) {
        erroSQL = query.lastError().text();
        db.close();
        return false;
    }
    db.close();
    return true;
}

bool Produto_Repository::updateAumentarQuantidadeProduto(qlonglong idprod, double quantia){
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao updateAumentarQuantidadeProduto";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("UPDATE produtos SET quantidade = quantidade + :quant WHERE id = :id");
    query.bindValue(":quant", quantia);
    query.bindValue(":id", idprod);

    if(!query.exec()){
        qDebug() << "Query updateAumentarQuantidadeProduto nao rodou.";
        db.close();
        return false;
    }else{
        db.close();
        return true;
    }
}

ProdutoDTO Produto_Repository::getProduto(qlonglong id){
    ProdutoDTO prod;
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao updateAumentarQuantidadeProduto";
        return prod;
    }

    QSqlQuery query(db);
    query.prepare("SELECT quantidade, descricao, preco, codigo_barras, nf, un_comercial, "
                  "preco_fornecedor, porcent_lucro, ncm, cest, aliquota_imposto, csosn, pis, local "
                  "FROM produtos WHERE id = :id");
    query.bindValue(":id", id);
    if(!query.exec()){
        qDebug() << "Query nao rodou getProduto()";
        db.close();
        return prod;
    }

    while(query.next()){
        prod.aliquotaIcms = query.value("aliquota_imposto").toDouble();
        prod.cest = query.value("cest").toString();
        prod.codigoBarras = query.value("codigo_barras").toString();
        prod.csosn = query.value("csosn").toString();
        prod.descricao = query.value("descricao").toString();
        prod.ncm = query.value("ncm").toString();
        prod.nf = query.value("nf").toBool();
        prod.percentLucro = query.value("porcent_lucro").toDouble();
        prod.pis = query.value("pis").toString();
        prod.preco = query.value("preco").toDouble();
        prod.precoFornecedor = query.value("preco_fornecedor").toDouble();
        prod.quantidade = query.value("quantidade").toDouble();
        prod.uCom = query.value("un_comercial").toString();
    }

    db.close();
    return prod;
}

ProdutoDTO Produto_Repository::getProdutoPeloCodBarras(const QString &codigo){
    ProdutoDTO prod;
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao getProdutoPeloCodBarras";
        return prod;
    }
    QSqlQuery query(db);
    query.prepare("SELECT * FROM produtos WHERE codigo_barras = :cod");
    query.bindValue(":cod", codigo);

    if(!query.exec()){
        qDebug() << " Query não executou getProdutoPeloCodBarras";
        db.close();
        return prod;
    }

    while(query.next()){
        prod.id = query.value("id").toLongLong();
        prod.quantidade = query.value("quantidade").toDouble();
        prod.descricao = query.value("descricao").toString();
        prod.preco =query.value("preco").toDouble();
        prod.codigoBarras = query.value("codigo_barras").toString();
        prod.nf = query.value("nf").toBool();
        prod.uCom = query.value("un_comercial").toString();
        prod.precoFornecedor = query.value("preco_fornecedor").toDouble();
        prod.percentLucro = query.value("porcent_lucro").toDouble();
        prod.ncm = query.value("ncm").toString();
        prod.cest = query.value("cest").toString();
        prod.aliquotaIcms = query.value("aliquota_imposto").toDouble();
        prod.csosn = query.value("csosn").toString();
        prod.pis = query.value("pis").toString();
        prod.local = query.value("local").toString();
    }

    db.close();
    return prod;
}

bool Produto_Repository::updateDiminuirQuantidadeProduto(qlonglong idprod, double quantia){
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao updateDiminuirQuantidadeProduto";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("UPDATE produtos SET quantidade = quantidade - :quant WHERE id = :id");
    query.bindValue(":quant", quantia);
    query.bindValue(":id", idprod);

    if(!query.exec()){
        qDebug() << "Query updateAumentarQuantidadeProduto nao rodou.";
        db.close();
        return false;
    }else{
        db.close();
        return true;
    }
}
