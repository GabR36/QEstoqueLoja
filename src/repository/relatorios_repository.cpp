#include "relatorios_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Relatorios_repository::Relatorios_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
}

QStringList Relatorios_repository::buscarAnosDisponiveis()
{
    QStringList anos;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarAnosDisponiveis";
        return anos;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT DISTINCT strftime('%Y', data_hora) AS ano
        FROM vendas2
        ORDER BY ano DESC
    )");

    if (query.exec()) {
        while (query.next()) {
            anos << query.value(0).toString();
        }
    } else {
        qDebug() << "Erro ao buscar anos:" << query.lastError().text();
    }

    db.close();
    return anos;
}

QMap<QString, int> Relatorios_repository::buscarVendasPorMes()
{
    QMap<QString, int> vendasPorMes;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarVendasPorMes";
        return vendasPorMes;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT strftime('%m', data_hora) AS mes, COUNT(*) AS total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = strftime('%Y', 'now')
        GROUP BY mes
    )");

    if (query.exec()) {
        while (query.next()) {
            QString mes = query.value(0).toString();
            int total = query.value(1).toInt();
            vendasPorMes[mes] = total;
        }
    } else {
        qDebug() << "Erro na consulta:" << query.lastError().text();
    }

    db.close();
    return vendasPorMes;
}

QMap<QString, int> Relatorios_repository::buscarVendasPorMesAno(const QString &ano)
{
    QMap<QString, int> vendasPorMes;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarVendasPorMesAno";
        return vendasPorMes;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT strftime('%m', data_hora) AS mes, COUNT(*) AS total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = :ano
        GROUP BY mes
    )");
    query.bindValue(":ano", ano);

    if (query.exec()) {
        while (query.next()) {
            QString mes = query.value(0).toString();
            int total = query.value(1).toInt();
            vendasPorMes[mes] = total;
        }
    } else {
        qDebug() << "Erro ao buscar vendas:" << query.lastError().text();
    }

    db.close();
    return vendasPorMes;
}

QMap<QString, int> Relatorios_repository::buscarVendasPorDiaMesAno(const QString &ano, const QString &mes)
{
    QMap<QString, int> vendasPorDia;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarVendasPorDiaMesAno";
        return vendasPorDia;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT strftime('%d', data_hora) AS dia, COUNT(*) AS total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = :ano AND strftime('%m', data_hora) = :mes
        GROUP BY dia
    )");
    query.bindValue(":ano", ano);
    query.bindValue(":mes", mes);

    if (query.exec()) {
        while (query.next()) {
            QString dia = query.value(0).toString();
            int total = query.value(1).toInt();
            vendasPorDia[dia] = total;
        }
    } else {
        qDebug() << "Erro ao buscar vendas por dia:" << query.lastError().text();
    }

    db.close();
    return vendasPorDia;
}

QMap<QString, double> Relatorios_repository::buscarValorVendasPorDiaMesAno(const QString &ano, const QString &mes)
{
    QMap<QString, double> vendasPorDia;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarValorVendasPorDiaMesAno";
        return vendasPorDia;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT strftime('%d', data_hora) AS dia, SUM(valor_final) AS total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = :ano
          AND strftime('%m', data_hora) = :mes
          AND forma_pagamento != 'Prazo'
        GROUP BY dia
    )");
    query.bindValue(":ano", ano);
    query.bindValue(":mes", mes);

    if (query.exec()) {
        while (query.next()) {
            QString dia = query.value(0).toString();
            double total = query.value(1).toDouble();
            vendasPorDia[dia] = total;
        }
    } else {
        qDebug() << "Erro ao buscar valor vendas por dia:" << query.lastError().text();
    }

    db.close();
    return vendasPorDia;
}

QMap<QString, QPair<double, double>> Relatorios_repository::buscarValorVendasPorMesAno(const QString &ano)
{
    QMap<QString, QPair<double, double>> totalPorMes;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarValorVendasPorMesAno";
        return totalPorMes;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT v.mes,
               COALESCE(v.total_vendas, 0) AS total_vendas,
               COALESCE(e.total_entradas, 0) AS total_entradas
        FROM
        (
            SELECT strftime('%m', data_hora) AS mes, SUM(valor_final) AS total_vendas
            FROM vendas2
            WHERE strftime('%Y', data_hora) = :ano AND forma_pagamento != 'Prazo'
            GROUP BY mes
        ) AS v
        LEFT JOIN
        (
            SELECT strftime('%m', data_hora) AS mes, SUM(valor_final) AS total_entradas
            FROM entradas_vendas
            WHERE strftime('%Y', data_hora) = :ano
            GROUP BY mes
        ) AS e
        ON v.mes = e.mes
    )");
    query.bindValue(":ano", ano);

    if (query.exec()) {
        while (query.next()) {
            QString mes = query.value(0).toString();
            double totalVendas = query.value(1).toDouble();
            double totalEntradas = query.value(2).toDouble();
            totalPorMes[mes] = QPair<double, double>(totalVendas, totalEntradas);
        }
    } else {
        qDebug() << "Erro ao buscar vendas e entradas:" << query.lastError().text();
    }

    db.close();
    return totalPorMes;
}

QMap<QString, int> Relatorios_repository::buscarTopProdutosVendidos()
{
    QMap<QString, int> topProdutos;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarTopProdutosVendidos";
        return topProdutos;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT p.descricao, SUM(pv.quantidade) AS total
        FROM produtos_vendidos pv
        JOIN produtos p ON pv.id_produto = p.id
        GROUP BY p.descricao
        ORDER BY total DESC
        LIMIT 10
    )");

    if (query.exec()) {
        while (query.next()) {
            QString produto = query.value(0).toString();
            int total = query.value(1).toInt();
            topProdutos[produto] = total;
        }
    } else {
        qDebug() << "Erro ao buscar top produtos:" << query.lastError().text();
    }

    db.close();
    return topProdutos;
}

QMap<QString, QVector<int>> Relatorios_repository::buscarFormasPagamentoPorAno(const QString &ano)
{
    QMap<QString, QVector<int>> resultado;

    QStringList formas = {"Dinheiro", "Crédito", "Débito", "Pix", "Prazo", "Não Sei"};
    for (const QString &forma : formas) {
        resultado[forma] = QVector<int>(12, 0);
    }

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarFormasPagamentoPorAno";
        return resultado;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT strftime('%m', data_hora) AS mes, forma_pagamento, COUNT(*) as total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = :ano
        GROUP BY mes, forma_pagamento
    )");
    query.bindValue(":ano", ano);

    if (query.exec()) {
        while (query.next()) {
            int mes = query.value(0).toString().toInt();
            QString forma = query.value(1).toString();
            int total = query.value(2).toInt();

            if (resultado.contains(forma)) {
                resultado[forma][mes - 1] = total;
            }
        }
    } else {
        qDebug() << "Erro ao buscar formas pagamento por ano:" << query.lastError().text();
    }

    db.close();
    return resultado;
}

QMap<QString, float> Relatorios_repository::buscarValoresNfAno(const QString &ano, int tpAmb)
{
    QMap<QString, float> valores;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. buscarValoresNfAno";
        return valores;
    }

    QSqlQuery query(db);
    query.prepare("SELECT strftime('%m', dhemi) AS mes, SUM(valor_total) "
                  "FROM notas_fiscais "
                  "WHERE (strftime('%Y', dhemi) = :ano "
                  "AND (cstat = '100' OR cstat = '150')) AND tp_amb = :tpamb "
                  "AND finalidade = 'NORMAL' "
                  "GROUP BY mes");
    query.bindValue(":ano", ano);
    query.bindValue(":tpamb", tpAmb);

    if (query.exec()) {
        while (query.next()) {
            QString mes = query.value(0).toString();
            float valor = query.value(1).toFloat();
            valores[mes] = valor;
        }
    } else {
        qDebug() << "Erro ao buscar valores das NFs por ano:" << query.lastError().text();
    }

    db.close();
    return valores;
}

QMap<QString, float> Relatorios_repository::produtosMaisLucrativosAno(const QString &ano)
{
    QMap<QString, float> produtosLucro;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco de dados. produtosMaisLucrativosAno";
        return produtosLucro;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT
            p.descricao,
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
        WHERE strftime('%Y', v.data_hora) = :ano
        GROUP BY p.descricao
        HAVING lucro_total > 0
        ORDER BY lucro_total DESC
        LIMIT 10
    )");
    query.bindValue(":ano", ano);

    if (query.exec()) {
        while (query.next()) {
            produtosLucro.insert(
                query.value(0).toString(),
                query.value(1).toFloat()
                );
        }
    } else {
        qDebug() << "Erro ao buscar produtos mais lucrativos:" << query.lastError().text();
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
        for (int i = 0; i < query.record().count(); ++i) {
            row << query.value(i).toString();
        }
        resultado << row;
    }

    db.close();
    return resultado;
}
