#include "rascunhovenda_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RascunhoVenda_repository::RascunhoVenda_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
}

bool RascunhoVenda_repository::salvar(const RascunhoVendaDTO &rascunho)
{
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (salvarRascunho)";
        return false;
    }


    QSqlQuery query(db);


    QString sql;


    if (DatabaseConnection_service::isPostgres()) {

        sql =
            "INSERT INTO rascunho_venda "
            "(id, id_cliente, cpf_manual, data_hora, produtos_json, "
            "forma_pagamento, desconto, taxa, recebido, desconto_porcentagem, "
            "modelo_nf, emitir_todos, atualizado_em) "

            "VALUES "
            "(1, :idCliente, :cpf, :dataHora, :json, "
            ":formaPag, :desconto, :taxa, :recebido, "
            ":descontoPorcentagem, :modeloNf, :emitirTodos, "
            ":atualizadoEm) "

            "ON CONFLICT (id) DO UPDATE SET "

            "id_cliente = EXCLUDED.id_cliente, "
            "cpf_manual = EXCLUDED.cpf_manual, "
            "data_hora = EXCLUDED.data_hora, "
            "produtos_json = EXCLUDED.produtos_json, "
            "forma_pagamento = EXCLUDED.forma_pagamento, "
            "desconto = EXCLUDED.desconto, "
            "taxa = EXCLUDED.taxa, "
            "recebido = EXCLUDED.recebido, "
            "desconto_porcentagem = EXCLUDED.desconto_porcentagem, "
            "modelo_nf = EXCLUDED.modelo_nf, "
            "emitir_todos = EXCLUDED.emitir_todos, "
            "atualizado_em = EXCLUDED.atualizado_em";


    } else {

        sql =
            "INSERT OR REPLACE INTO rascunho_venda "
            "(id, id_cliente, cpf_manual, data_hora, produtos_json, "
            "forma_pagamento, desconto, taxa, recebido, desconto_porcentagem, "
            "modelo_nf, emitir_todos, atualizado_em) "

            "VALUES "
            "(1, :idCliente, :cpf, :dataHora, :json, "
            ":formaPag, :desconto, :taxa, :recebido, "
            ":descontoPorcentagem, :modeloNf, :emitirTodos, "
            ":atualizadoEm)";
    }


    if (!query.prepare(sql)) {

        qDebug()
        << "Erro prepare salvar rascunho:"
        << query.lastError().text();

        db.close();

        return false;
    }


    query.bindValue(":idCliente", rascunho.idCliente);
    query.bindValue(":cpf", rascunho.cpfManual);
    query.bindValue(":dataHora", rascunho.dataHora);
    query.bindValue(":json", rascunho.produtosJson);
    query.bindValue(":formaPag", rascunho.formaPagamento);

    query.bindValue(":desconto",
                    rascunho.desconto.isEmpty()
                        ? 0
                        : rascunho.desconto.toDouble());

    query.bindValue(":taxa",
                    rascunho.taxa.isEmpty()
                        ? 0
                        : rascunho.taxa.toDouble());

    query.bindValue(":recebido",
                    rascunho.recebido.isEmpty()
                        ? 0
                        : rascunho.recebido.toDouble());

    query.bindValue(":descontoPorcentagem",
                    rascunho.descontoPorcentagem ? 1 : 0);

    query.bindValue(":modeloNf", rascunho.modeloNf);

    query.bindValue(":emitirTodos",
                    rascunho.emitirTodos ? 1 : 0);

    query.bindValue(":atualizadoEm",
                    rascunho.atualizadoEm);


    if (!query.exec()) {

        qDebug()
        << "Erro ao salvar rascunho:"
        << query.lastError().text();

        db.close();

        return false;
    }


    db.close();

    return true;
}

bool RascunhoVenda_repository::descartar()
{
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (descartarRascunho)";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM rascunho_venda WHERE id = 1");

    if (!query.exec()) {
        qDebug() << "Erro ao descartar rascunho:" << query.lastError().text();
        db.close();
        return false;
    }

    db.close();
    return true;
}

RascunhoVendaDTO RascunhoVenda_repository::carregar()
{
    RascunhoVendaDTO rascunho;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (carregarRascunho)";
        return rascunho;
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT id_cliente, cpf_manual, data_hora, produtos_json, "
        "       forma_pagamento, desconto, taxa, recebido, desconto_porcentagem, "
        "       modelo_nf, emitir_todos, atualizado_em "
        "FROM rascunho_venda WHERE id = 1"
    );

    if (!query.exec()) {
        qDebug() << "Erro ao carregar rascunho:" << query.lastError().text();
        db.close();
        return rascunho;
    }

    if (query.next()) {
        rascunho.idCliente           = query.value("id_cliente").toLongLong();
        rascunho.cpfManual           = query.value("cpf_manual").toString();
        rascunho.dataHora            = query.value("data_hora").toString();
        rascunho.produtosJson        = query.value("produtos_json").toString();
        rascunho.formaPagamento      = query.value("forma_pagamento").toString();
        rascunho.desconto            = query.value("desconto").toString();
        rascunho.taxa                = query.value("taxa").toString();
        rascunho.recebido            = query.value("recebido").toString();
        rascunho.descontoPorcentagem = query.value("desconto_porcentagem").toInt() == 1;
        rascunho.modeloNf            = query.value("modelo_nf").toInt();
        rascunho.emitirTodos         = query.value("emitir_todos").toInt() == 1;
        rascunho.atualizadoEm        = query.value("atualizado_em").toString();
    }

    db.close();
    return rascunho;
}

bool RascunhoVenda_repository::existe()
{
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (existeRascunho)";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM rascunho_venda WHERE id = 1");

    if (!query.exec() || !query.next()) {
        db.close();
        return false;
    }

    bool result = query.value(0).toInt() > 0;
    db.close();
    return result;
}
