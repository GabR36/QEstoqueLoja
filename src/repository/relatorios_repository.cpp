#include "relatorios_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlRecord>
#include <QDateTime>

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

QString Relatorios_repository::dateFormatSql(Agrupamento agrup)
{
    return dateFormatSql(agrup, "data_hora");
}

QString Relatorios_repository::dateFormatSql(
    Agrupamento agrup,
    const QString &campo)
{
    bool postgres = db.driverName().contains("QPSQL");

    if (postgres) {

        QString campoData =
            QString("CAST(%1 AS TIMESTAMP)").arg(campo);

        switch (agrup) {

        case Agrupamento::Dia:
            return QString("TO_CHAR(%1, 'YYYY-MM-DD')")
                .arg(campoData);

        case Agrupamento::Mes:
            return QString("TO_CHAR(%1, 'YYYY-MM')")
                .arg(campoData);

        case Agrupamento::Ano:
            return QString("TO_CHAR(%1, 'YYYY')")
                .arg(campoData);
        }

    } else {

        switch (agrup) {

        case Agrupamento::Dia:
            return QString("strftime('%Y-%m-%d', %1)")
                .arg(campo);

        case Agrupamento::Mes:
            return QString("strftime('%Y-%m', %1)")
                .arg(campo);

        case Agrupamento::Ano:
            return QString("strftime('%Y', %1)")
                .arg(campo);
        }
    }

    return "";
}

QMap<QString, int> Relatorios_repository::buscarQuantVendasPeriodo(
    const QDate &inicio,
    const QDate &fim,
    Agrupamento agrup)
{
    QMap<QString, int> resultado;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarQuantVendasPeriodo";
        return resultado;
    }

    QString fmt = dateFormatSql(agrup);

    bool postgres = db.driverName().contains("QPSQL");

    QString filtroData = postgres
                             ? "data_hora::date"
                             : "date(data_hora)";


    QSqlQuery query(db);

    query.prepare(QString(R"(
        SELECT %1 AS periodo,
               COUNT(*) AS total
        FROM vendas2
        WHERE %2 BETWEEN :inicio AND :fim
        GROUP BY %1
        ORDER BY periodo
    )").arg(fmt, filtroData));


    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim", fim.toString(Qt::ISODate));


    if (query.exec()) {

        while (query.next()) {
            resultado[
                query.value("periodo").toString()
            ] =
                query.value("total").toInt();
        }

    } else {

        qDebug()
        << "Erro buscarQuantVendasPeriodo:"
        << query.lastError().text();
    }



    return resultado;
}

QMap<QString, QPair<double,double>> Relatorios_repository::buscarValorVendasPeriodo(
    const QDate &inicio,
    const QDate &fim,
    Agrupamento agrup)
{
    QMap<QString, QPair<double,double>> resultado;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarValorVendasPeriodo";
        return resultado;
    }

    QString fmt = dateFormatSql(agrup);

    bool postgres = db.driverName().contains("QPSQL");

    QString filtroData = postgres
                             ? "data_hora::date"
                             : "date(data_hora)";

    QSqlQuery query(db);

    query.prepare(QString(R"(
        SELECT v.periodo,
               COALESCE(v.total_vendas, 0),
               COALESCE(e.total_entradas, 0)
        FROM (
            SELECT %1 AS periodo,
                   SUM(valor_final) AS total_vendas
            FROM vendas2
            WHERE %2 BETWEEN :inicio1 AND :fim1
              AND forma_pagamento <> 'Prazo'
            GROUP BY %1
        ) AS v
        LEFT JOIN (
            SELECT %1 AS periodo,
                   SUM(valor_final) AS total_entradas
            FROM entradas_vendas
            WHERE %2 BETWEEN :inicio2 AND :fim2
            GROUP BY %1
        ) AS e
        ON v.periodo = e.periodo
        ORDER BY v.periodo
    )").arg(fmt, filtroData));


    query.bindValue(":inicio1", inicio.toString(Qt::ISODate));
    query.bindValue(":fim1",    fim.toString(Qt::ISODate));

    query.bindValue(":inicio2", inicio.toString(Qt::ISODate));
    query.bindValue(":fim2",    fim.toString(Qt::ISODate));


    if (query.exec()) {

        while (query.next()) {

            QString periodo = query.value(0).toString();

            double vendas =
                query.value(1).toDouble();

            double entradas =
                query.value(2).toDouble();


            resultado[periodo] =
                QPair<double,double>(vendas, entradas);
        }

    } else {

        qDebug()
        << "Erro buscarValorVendasPeriodo:"
        << query.lastError().text();
    }



    return resultado;
}

QMap<QString, int> Relatorios_repository::buscarTopProdutosVendidosPeriodo(
    const QDate &inicio,
    const QDate &fim)
{
    QMap<QString, int> topProdutos;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarTopProdutosVendidosPeriodo";
        return topProdutos;
    }

    bool postgres = db.driverName().contains("QPSQL");
    QString filtroData = postgres
                             ? "v.data_hora::date"
                             : "date(v.data_hora)";

    QSqlQuery query(db);

    query.prepare(QString(R"(
        SELECT p.descricao,
               SUM(pv.quantidade) AS total
        FROM produtos_vendidos pv
        JOIN produtos p
             ON pv.id_produto = p.id
        JOIN vendas2 v
             ON pv.id_venda = v.id
        WHERE %1 BETWEEN :inicio AND :fim
        GROUP BY p.descricao
        ORDER BY total DESC
        LIMIT 10
    )").arg(filtroData));

    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            topProdutos[
                query.value(0).toString()
            ] =
                query.value(1).toInt();
        }
    } else {
        qDebug()
        << "Erro buscarTopProdutosVendidosPeriodo:"
        << query.lastError().text();
    }


    return topProdutos;
}

QMap<QString, QMap<QString,int>> Relatorios_repository::buscarFormasPagamentoPeriodo(
    const QDate &inicio,
    const QDate &fim,
    Agrupamento agrup)
{
    QMap<QString, QMap<QString,int>> resultado;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarFormasPagamentoPeriodo";
        return resultado;
    }

    QString fmt = dateFormatSql(agrup);

    bool postgres = db.driverName().contains("QPSQL");

    QString filtroData = postgres
                             ? "data_hora::date"
                             : "date(data_hora)";

    QSqlQuery query(db);

    query.prepare(QString(R"(
        SELECT forma_pagamento,
               %1 AS periodo,
               COUNT(*) AS total
        FROM vendas2
        WHERE %2 BETWEEN :inicio AND :fim
        GROUP BY forma_pagamento, %1
        ORDER BY periodo
    )").arg(fmt, filtroData));

    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));
    if (query.exec()) {
        while (query.next()) {
            QString forma =
                query.value(0).toString();
            QString periodo =
                query.value(1).toString();
            int total =
                query.value(2).toInt();
            resultado[forma][periodo] = total;
        }
    } else {
        qDebug()
        << "Erro buscarFormasPagamentoPeriodo:"
        << query.lastError().text();
    }
    return resultado;
}

QMap<QString, float> Relatorios_repository::buscarValoresNfPeriodo(
    const QDate &inicio,
    const QDate &fim,
    Agrupamento agrup,
    int tpAmb)
{
    QMap<QString, float> valores;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarValoresNfPeriodo";
        return valores;
    }
    QString fmt = dateFormatSql(agrup, "dhemi");

    bool postgres = db.driverName().contains("QPSQL");

    QString filtroData = postgres
                             ? "CAST(CAST(dhemi AS TIMESTAMP) AS DATE)"
                             : "date(dhemi)";

    QSqlQuery query(db);

    query.prepare(QString(
                      R"(
        SELECT %1 AS periodo,
               SUM(
                   CASE
                       WHEN finalidade = 'DEVOLUCAO'
                       THEN -valor_total
                       ELSE valor_total
                   END
               )
        FROM notas_fiscais
        WHERE %2 BETWEEN :inicio AND :fim
          AND (cstat = '100' OR cstat = '150')
          AND tp_amb = :tpamb
          AND finalidade IN ('NORMAL', 'DEVOLUCAO')
        GROUP BY %1
        ORDER BY periodo
        )"
                      ).arg(fmt, filtroData));


    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));
    query.bindValue(":tpamb",  tpAmb);
    qDebug() << query.lastQuery();
    if (query.exec()) {
        while (query.next()) {

            valores[
                query.value(0).toString()
            ] =
                query.value(1).toFloat();
        }
    } else {

        qDebug()
        << "Erro buscarValoresNfPeriodo:"
        << query.lastError().text();
    }

    // db.close();

    return valores;
}

QMap<QString, float> Relatorios_repository::produtosMaisLucrativosPeriodo(
    const QDate &inicio,
    const QDate &fim)
{
    QMap<QString, float> produtosLucro;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. produtosMaisLucrativosPeriodo";
        return produtosLucro;
    }

    bool postgres = db.driverName().contains("QPSQL");
    QString filtroData = postgres
                             ? "v.data_hora::date"
                             : "date(v.data_hora)";

    QSqlQuery query(db);
    query.prepare(QString(R"(
        SELECT p.descricao,
               SUM(
                   pv.quantidade *
                   CASE
                       WHEN p.preco_fornecedor IS NOT NULL
                            AND p.preco_fornecedor > 0
                       THEN
                           (pv.preco_vendido - p.preco_fornecedor)
                       ELSE
                           (pv.preco_vendido * (p.porcent_lucro / 100.0))
                   END
               ) AS lucro_total
        FROM produtos_vendidos pv
        JOIN produtos p
             ON pv.id_produto = p.id
        JOIN vendas2 v
             ON pv.id_venda = v.id
        WHERE %1 BETWEEN :inicio AND :fim
        GROUP BY p.descricao
        HAVING SUM(
                   pv.quantidade *
                   CASE
                       WHEN p.preco_fornecedor IS NOT NULL
                            AND p.preco_fornecedor > 0
                       THEN
                           (pv.preco_vendido - p.preco_fornecedor)
                       ELSE
                           (pv.preco_vendido * (p.porcent_lucro / 100.0))
                   END
               ) > 0
        ORDER BY lucro_total DESC
        LIMIT 10
    )").arg(filtroData));

    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            produtosLucro.insert(
                query.value(0).toString(),
                query.value(1).toFloat()
                );
        }
    } else {
        qDebug()
        << "Erro produtosMaisLucrativosPeriodo:"
        << query.lastError().text();
    }

    return produtosLucro;
}

QMap<QString, double> Relatorios_repository::buscarLucroPeriodo(
    const QDate &inicio,
    const QDate &fim,
    Agrupamento agrup)
{
    QMap<QString, double> resultado;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarLucroPeriodo";
        return resultado;
    }

    bool postgres = db.driverName().contains("QPSQL");

    QString fmtV  = dateFormatSql(agrup, "v.data_hora");
    QString fmtEV = dateFormatSql(agrup, "ev.data_hora");
    QString filtroV  = postgres ? "v.data_hora::date"   : "date(v.data_hora)";
    QString filtroEV = postgres ? "ev.data_hora::date"  : "date(ev.data_hora)";

    // custo_unitario   = p.preco * (1 - porcent_lucro/100)
    // lucro_item       = (pv.preco_vendido - custo_unitario) * quantidade
    // lucro_total_venda = SUM(lucro_item) -> lucro bruto da venda, sem desconto/taxa
    // taxa_valor       = (total_pago - desconto_pago) * taxa/100 (repassada ao cartão,
    //                    só quando a forma de pagamento daquele pagamento é credito/debito)
    //
    // vendas a vista (cash): a venda inteira é paga em uma unica tacada, entao o
    // desconto dado na venda e a taxa daquele pagamento sao abatidos direto do lucro bruto.
    //
    // vendas a prazo (credit/entradas_vendas): o desconto foi concedido uma unica vez, no
    // momento da venda original (vendas2.desconto), mas o dinheiro chega em varias entradas
    // ao longo do tempo. Por isso o desconto e rateado proporcionalmente ao quanto cada
    // entrada representa do valor final total devido (ev.valor_final / v.valor_final),
    // enquanto a taxa de cada entrada é abatida integralmente, pois é especifica daquele
    // pagamento (cada entrada pode ter sua propria forma de pagamento/taxa).
    QSqlQuery query(db);
    query.prepare(QString(R"(
        WITH sale_stats AS (
            SELECT
                pv.id_venda,
                SUM(pv.quantidade * pv.preco_vendido) AS valor_total,
                SUM(
                    pv.quantidade * (
                        pv.preco_vendido
                        - p.preco * (1.0 - COALESCE(p.porcent_lucro, 0) / 100.0)
                    )
                ) AS lucro_total
            FROM produtos_vendidos pv
            JOIN produtos p ON pv.id_produto = p.id
            GROUP BY pv.id_venda
        ),
        cash AS (
            SELECT %1 AS periodo,
                   SUM(
                       ss.lucro_total
                       - COALESCE(v.desconto, 0)
                       - CASE
                             WHEN v.forma_pagamento IN ('Crédito', 'Débito')
                             THEN (v.total - COALESCE(v.desconto, 0)) * COALESCE(v.taxa, 0) / 100.0
                             ELSE 0
                         END
                   ) AS lucro_caixa
            FROM vendas2 v
            JOIN sale_stats ss ON ss.id_venda = v.id
            WHERE %3 BETWEEN :inicio1 AND :fim1
              AND v.forma_pagamento <> 'Prazo'
            GROUP BY %1
        ),
        credit AS (
            SELECT %2 AS periodo,
                   SUM(
                       (ss.lucro_total - COALESCE(v.desconto, 0))
                       * ev.valor_final / NULLIF(v.valor_final, 0)
                       - CASE
                             WHEN ev.forma_pagamento IN ('Crédito', 'Débito')
                             THEN (ev.total - COALESCE(ev.desconto, 0)) * COALESCE(ev.taxa, 0) / 100.0
                             ELSE 0
                         END
                   ) AS lucro_caixa
            FROM entradas_vendas ev
            JOIN sale_stats ss ON ss.id_venda = ev.id_venda
            JOIN vendas2 v ON v.id = ev.id_venda
            WHERE %4 BETWEEN :inicio2 AND :fim2
            GROUP BY %2
        )
        SELECT periodo, SUM(lucro_caixa) AS total_lucro
        FROM (
            SELECT periodo, lucro_caixa FROM cash
            UNION ALL
            SELECT periodo, lucro_caixa FROM credit
        ) combined
        GROUP BY periodo
        ORDER BY periodo
    )").arg(fmtV, fmtEV, filtroV, filtroEV));

    query.bindValue(":inicio1", inicio.toString(Qt::ISODate));
    query.bindValue(":fim1",    fim.toString(Qt::ISODate));
    query.bindValue(":inicio2", inicio.toString(Qt::ISODate));
    query.bindValue(":fim2",    fim.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            resultado[query.value(0).toString()] = query.value(1).toDouble();
        }
    } else {
        qDebug() << "Erro buscarLucroPeriodo:" << query.lastError().text();
    }

    return resultado;
}

double Relatorios_repository::buscarLucroVenda(qlonglong idVenda)
{
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarLucroVenda";
        return 0.0;
    }

    // mesma logica de buscarLucroPeriodo, mas para uma unica venda: uma so
    // query, sem agrupar por periodo, para nao gerar N+1 quando chamada em lote.
    QSqlQuery query(db);
    query.prepare(R"(
        WITH sale_stats AS (
            SELECT
                pv.id_venda,
                SUM(
                    pv.quantidade * (
                        pv.preco_vendido
                        - p.preco * (1.0 - COALESCE(p.porcent_lucro, 0) / 100.0)
                    )
                ) AS lucro_total
            FROM produtos_vendidos pv
            JOIN produtos p ON pv.id_produto = p.id
            WHERE pv.id_venda = :idVenda1
            GROUP BY pv.id_venda
        ),
        cash AS (
            SELECT
                ss.lucro_total
                - COALESCE(v.desconto, 0)
                - CASE
                      WHEN v.forma_pagamento IN ('Crédito', 'Débito')
                      THEN (v.total - COALESCE(v.desconto, 0)) * COALESCE(v.taxa, 0) / 100.0
                      ELSE 0
                  END AS lucro_caixa
            FROM vendas2 v
            JOIN sale_stats ss ON ss.id_venda = v.id
            WHERE v.id = :idVenda2
              AND v.forma_pagamento <> 'Prazo'
        ),
        credit AS (
            SELECT
                (ss.lucro_total - COALESCE(v.desconto, 0))
                * ev.valor_final / NULLIF(v.valor_final, 0)
                - CASE
                      WHEN ev.forma_pagamento IN ('Crédito', 'Débito')
                      THEN (ev.total - COALESCE(ev.desconto, 0)) * COALESCE(ev.taxa, 0) / 100.0
                      ELSE 0
                  END AS lucro_caixa
            FROM entradas_vendas ev
            JOIN sale_stats ss ON ss.id_venda = ev.id_venda
            JOIN vendas2 v ON v.id = ev.id_venda
            WHERE ev.id_venda = :idVenda3
        )
        SELECT COALESCE(SUM(lucro_caixa), 0)
        FROM (
            SELECT lucro_caixa FROM cash
            UNION ALL
            SELECT lucro_caixa FROM credit
        ) combined
    )");

    query.bindValue(":idVenda1", idVenda);
    query.bindValue(":idVenda2", idVenda);
    query.bindValue(":idVenda3", idVenda);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    qDebug() << "Erro buscarLucroVenda:" << query.lastError().text();
    return 0.0;
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
            return true;
        }
    } else {
        qDebug() << "Erro na consulta:" << query.lastError().text();
    }

    return false;
}

QList<QStringList> Relatorios_repository::buscarInventario(
    const QDate &inicio,
    const QDate &fim,
    bool somenteNf)
{
    QList<QStringList> resultado;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarInventario";
        return resultado;
    }

    bool postgres = db.driverName().contains("QPSQL");

    QString filtroData = postgres
                             ? "adicionado_em::date"
                             : "date(adicionado_em)";

    QSqlQuery query(db);

    query.prepare(QString(R"(
        SELECT id,
               quantidade,
               descricao,
               un_comercial,
               CASE
                   WHEN preco_fornecedor IS NOT NULL
                        AND preco_fornecedor > 0
                   THEN preco_fornecedor
                   ELSE preco / (1.0 + porcent_lucro / 100.0)
               END AS preco_custo
        FROM produtos
        WHERE (:somente_nf = 0 OR nf = 1)
          AND quantidade > 0
          AND %1 BETWEEN :inicio AND :fim
        ORDER BY descricao
    )").arg(filtroData));

    query.bindValue(":somente_nf", somenteNf ? 1 : 0);
    query.bindValue(":inicio", inicio.toString(Qt::ISODate));
    query.bindValue(":fim",    fim.toString(Qt::ISODate));

    if (!query.exec()) {
        qDebug()
        << "Erro buscarInventario:"
        << query.lastError().text();
        return resultado;
    }

    while (query.next()) {
        resultado << QStringList{
            query.value("id").toString(),
                query.value("quantidade").toString(),
                query.value("descricao").toString(),
                query.value("un_comercial").toString(),
                QString::number(
                    query.value("preco_custo").toDouble(),
                    'f',
                    2
                    )
        };
    }

    return resultado;
}

QList<QStringList> Relatorios_repository::buscarClientesInadimplentes()
{
    QList<QStringList> resultado;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarClientesInadimplentes";
        return resultado;
    }

    QSqlQuery query(db);

    // data_referencia = data do ultimo pagamento (entradas_vendas), ou,
    // se o cliente nunca pagou nada, a data da compra a prazo mais antiga.
    // Ordenar por essa data ASC coloca quem esta ha mais tempo sem pagar primeiro.
    query.prepare(R"(
        WITH devido AS (
            SELECT id_cliente, SUM(valor_final) AS total_devido
            FROM vendas2
            WHERE forma_pagamento = 'Prazo'
            GROUP BY id_cliente
        ),
        pago AS (
            SELECT v.id_cliente, SUM(ev.valor_final) AS total_pago
            FROM entradas_vendas ev
            JOIN vendas2 v ON v.id = ev.id_venda
            GROUP BY v.id_cliente
        ),
        ultimo_pagamento AS (
            SELECT v.id_cliente, MAX(ev.data_hora) AS data_pagamento
            FROM entradas_vendas ev
            JOIN vendas2 v ON v.id = ev.id_venda
            GROUP BY v.id_cliente
        ),
        primeira_compra AS (
            SELECT id_cliente, MIN(data_hora) AS data_compra
            FROM vendas2
            WHERE forma_pagamento = 'Prazo'
            GROUP BY id_cliente
        )
        SELECT c.nome,
               c.telefone,
               (d.total_devido - COALESCE(p.total_pago, 0)) AS valor_devido,
               COALESCE(up.data_pagamento, pc.data_compra) AS data_referencia
        FROM clientes c
        JOIN devido d ON d.id_cliente = c.id
        LEFT JOIN pago p ON p.id_cliente = c.id
        LEFT JOIN ultimo_pagamento up ON up.id_cliente = c.id
        JOIN primeira_compra pc ON pc.id_cliente = c.id
        WHERE (d.total_devido - COALESCE(p.total_pago, 0)) > 0.005
        ORDER BY data_referencia ASC
    )");

    if (!query.exec()) {
        qDebug()
        << "Erro buscarClientesInadimplentes:"
        << query.lastError().text();
        return resultado;
    }

    QDate hoje = QDate::currentDate();

    while (query.next()) {
        QString nome = query.value(0).toString();
        QString telefone = query.value(1).toString();
        double valorDevido = query.value(2).toDouble();

        QString dataRefStr = query.value(3).toString();
        QDateTime dataRef = QDateTime::fromString(dataRefStr, "yyyy-MM-dd HH:mm:ss");
        if (!dataRef.isValid())
            dataRef = QDateTime::fromString(dataRefStr, Qt::ISODate);

        QString dataRefFormatada = dataRef.isValid()
                                        ? dataRef.date().toString("dd/MM/yyyy")
                                        : dataRefStr;
        QString diasSemPagar = dataRef.isValid()
                                    ? QString::number(dataRef.date().daysTo(hoje))
                                    : "";

        resultado << QStringList{
            nome,
            telefone,
            QString::number(valorDevido, 'f', 2),
            dataRefFormatada,
            diasSemPagar
        };
    }

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
        return resultado;
    }

    while (query.next()) {
        QStringList row;
        for (int i = 0; i < query.record().count(); ++i)
            row << query.value(i).toString();
        resultado << row;
    }

    return resultado;
}
