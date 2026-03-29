#include "relatorios_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlRecord>

Relatorios_repository::Relatorios_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
}

QString Relatorios_repository::strftimeFormato(Agrupamento agrup)
{
    switch (agrup) {
    case Agrupamento::Dia: return "%Y-%m-%d";
    case Agrupamento::Mes: return "%Y-%m";
    case Agrupamento::Ano: return "%Y";
    }
    return "%Y-%m";
}

QMap<QString, int> Relatorios_repository::buscarQuantVendasPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup)
{
    QMap<QString, int> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarQuantVendasPeriodo";
        return resultado;
    }

    QString fmt = strftimeFormato(agrup);
    QSqlQuery query(db);
    query.prepare(QString(R"(
        SELECT strftime('%1', data_hora) AS periodo, COUNT(*) AS total
        FROM vendas2
        WHERE date(data_hora) BETWEEN :inicio AND :fim
        GROUP BY periodo
        ORDER BY periodo
    )").arg(fmt));
    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next())
            resultado[query.value(0).toString()] = query.value(1).toInt();
    } else {
        qDebug() << "Erro buscarQuantVendasPeriodo:" << query.lastError().text();
    }
    db.close();
    return resultado;
}

QMap<QString, QPair<double,double>> Relatorios_repository::buscarValorVendasPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup)
{
    QMap<QString, QPair<double,double>> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarValorVendasPeriodo";
        return resultado;
    }

    QString fmt = strftimeFormato(agrup);
    QSqlQuery query(db);
    query.prepare(QString(R"(
        SELECT v.periodo,
               COALESCE(v.total_vendas, 0),
               COALESCE(e.total_entradas, 0)
        FROM (
            SELECT strftime('%1', data_hora) AS periodo, SUM(valor_final) AS total_vendas
            FROM vendas2
            WHERE date(data_hora) BETWEEN :inicio1 AND :fim1
              AND forma_pagamento != 'Prazo'
            GROUP BY periodo
        ) AS v
        LEFT JOIN (
            SELECT strftime('%1', data_hora) AS periodo, SUM(valor_final) AS total_entradas
            FROM entradas_vendas
            WHERE date(data_hora) BETWEEN :inicio2 AND :fim2
            GROUP BY periodo
        ) AS e ON v.periodo = e.periodo
        ORDER BY v.periodo
    )").arg(fmt));
    query.bindValue(":inicio1", inicio.toString(Qt::ISODate));
    query.bindValue(":fim1",    fim.toString(Qt::ISODate));
    query.bindValue(":inicio2", inicio.toString(Qt::ISODate));
    query.bindValue(":fim2",    fim.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            QString periodo  = query.value(0).toString();
            double  vendas   = query.value(1).toDouble();
            double  entradas = query.value(2).toDouble();
            resultado[periodo] = QPair<double,double>(vendas, entradas);
        }
    } else {
        qDebug() << "Erro buscarValorVendasPeriodo:" << query.lastError().text();
    }
    db.close();
    return resultado;
}

QMap<QString, int> Relatorios_repository::buscarTopProdutosVendidosPeriodo(const QDate &inicio, const QDate &fim)
{
    QMap<QString, int> topProdutos;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarTopProdutosVendidosPeriodo";
        return topProdutos;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT p.descricao, SUM(pv.quantidade) AS total
        FROM produtos_vendidos pv
        JOIN produtos p ON pv.id_produto = p.id
        JOIN vendas2 v ON pv.id_venda = v.id
        WHERE date(v.data_hora) BETWEEN :inicio AND :fim
        GROUP BY p.descricao
        ORDER BY total DESC
        LIMIT 10
    )");
    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next())
            topProdutos[query.value(0).toString()] = query.value(1).toInt();
    } else {
        qDebug() << "Erro buscarTopProdutosVendidosPeriodo:" << query.lastError().text();
    }
    db.close();
    return topProdutos;
}

QMap<QString, QMap<QString,int>> Relatorios_repository::buscarFormasPagamentoPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup)
{
    QMap<QString, QMap<QString,int>> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarFormasPagamentoPeriodo";
        return resultado;
    }

    QString fmt = strftimeFormato(agrup);
    QSqlQuery query(db);
    query.prepare(QString(R"(
        SELECT forma_pagamento, strftime('%1', data_hora) AS periodo, COUNT(*) AS total
        FROM vendas2
        WHERE date(data_hora) BETWEEN :inicio AND :fim
        GROUP BY forma_pagamento, periodo
        ORDER BY periodo
    )").arg(fmt));
    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            QString forma   = query.value(0).toString();
            QString periodo = query.value(1).toString();
            int     total   = query.value(2).toInt();
            resultado[forma][periodo] = total;
        }
    } else {
        qDebug() << "Erro buscarFormasPagamentoPeriodo:" << query.lastError().text();
    }
    db.close();
    return resultado;
}

QMap<QString, float> Relatorios_repository::buscarValoresNfPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup, int tpAmb)
{
    QMap<QString, float> valores;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarValoresNfPeriodo";
        return valores;
    }

    QString fmt = strftimeFormato(agrup);
    QSqlQuery query(db);
    query.prepare(QString(
        "SELECT strftime('%1', dhemi) AS periodo, "
        "SUM(CASE WHEN finalidade = 'DEVOLUCAO' THEN -valor_total ELSE valor_total END) "
        "FROM notas_fiscais "
        "WHERE date(dhemi) BETWEEN :inicio AND :fim "
        "  AND (cstat = '100' OR cstat = '150') AND tp_amb = :tpamb "
        "  AND finalidade IN ('NORMAL', 'DEVOLUCAO') "
        "GROUP BY periodo "
        "ORDER BY periodo"
    ).arg(fmt));
    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));
    query.bindValue(":tpamb",  tpAmb);

    if (query.exec()) {
        while (query.next())
            valores[query.value(0).toString()] = query.value(1).toFloat();
    } else {
        qDebug() << "Erro buscarValoresNfPeriodo:" << query.lastError().text();
    }
    db.close();
    return valores;
}

QMap<QString, float> Relatorios_repository::produtosMaisLucrativosPeriodo(const QDate &inicio, const QDate &fim)
{
    QMap<QString, float> produtosLucro;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. produtosMaisLucrativosPeriodo";
        return produtosLucro;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT p.descricao,
               SUM(
                   pv.quantidade *
                   CASE
                       WHEN p.preco_fornecedor > 0 AND p.preco_fornecedor != '' THEN
                           (pv.preco_vendido - p.preco_fornecedor)
                       ELSE
                           (pv.preco_vendido * (p.porcent_lucro / 100.0))
                   END
               ) AS lucro_total
        FROM produtos_vendidos pv
        JOIN produtos p ON pv.id_produto = p.id
        JOIN vendas2 v ON pv.id_venda = v.id
        WHERE date(v.data_hora) BETWEEN :inicio AND :fim
        GROUP BY p.descricao
        HAVING lucro_total > 0
        ORDER BY lucro_total DESC
        LIMIT 10
    )");
    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next())
            produtosLucro.insert(query.value(0).toString(), query.value(1).toFloat());
    } else {
        qDebug() << "Erro produtosMaisLucrativosPeriodo:" << query.lastError().text();
    }
    db.close();
    return produtosLucro;
}

bool Relatorios_repository::existeProdutoVendido()
{
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. existeProdutoVendido";
        return false;
    }

    QSqlQuery query(db);
    if (query.exec("SELECT 1 FROM produtos_vendidos LIMIT 1")) {
        if (query.next()) {
            db.close();
            return true;
        }
    } else {
        qDebug() << "Erro na consulta:" << query.lastError().text();
    }

    db.close();
    return false;
}

QList<QStringList> Relatorios_repository::buscarInventario(const QDate &inicio, const QDate &fim, bool somenteNf)
{
    QList<QStringList> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarInventario";
        return resultado;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, quantidade, descricao, un_comercial,
               CASE
                   WHEN preco_fornecedor IS NOT NULL AND preco_fornecedor > 0
                       THEN preco_fornecedor
                   ELSE preco / (1.0 + porcent_lucro / 100.0)
               END AS preco_custo
        FROM produtos
        WHERE (:somente_nf = 0 OR nf = 1)
          AND quantidade > 0
          AND date(adicionado_em) BETWEEN :inicio AND :fim
        ORDER BY descricao
    )");
    query.bindValue(":somente_nf", somenteNf ? 1 : 0);
    query.bindValue(":inicio",     inicio.toString(Qt::ISODate));
    query.bindValue(":fim",        fim.toString(Qt::ISODate));

    if (!query.exec()) {
        qDebug() << "Erro buscarInventario:" << query.lastError().text();
        db.close();
        return resultado;
    }

    while (query.next()) {
        resultado << QStringList{
            query.value("id").toString(),
            query.value("quantidade").toString(),
            query.value("descricao").toString(),
            query.value("un_comercial").toString(),
            QString::number(query.value("preco_custo").toDouble(), 'f', 2)
        };
    }

    db.close();
    return resultado;
}

QList<QStringList> Relatorios_repository::buscarTodosProdutosParaCsv()
{
    QList<QStringList> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarTodosProdutosParaCsv";
        return resultado;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM produtos")) {
        qDebug() << "Erro ao buscar produtos para CSV:" << query.lastError().text();
        db.close();
        return resultado;
    }

    while (query.next()) {
        QStringList row;
        for (int i = 0; i < query.record().count(); ++i)
            row << query.value(i).toString();
        resultado << row;
    }

    db.close();
    return resultado;
}
