#include "contingencia_repository.h"
#include "../util/datautil.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "../infra/databaseconnection_service.h"
#include <QUuid>

ContingenciaRepository::ContingenciaRepository(const QSqlDatabase &db, QObject *parent)
    : QObject{parent}
    , m_db{db}
{}

QList<NotaFiscalDTO> ContingenciaRepository::buscarPendentes()
{
    QList<NotaFiscalDTO> lista;

    QSqlQuery q(m_db);
    q.prepare("SELECT cstat, nnf, serie, modelo, tp_amb, xml_path, valor_total, "
              "id_venda, cnpjemit, chnfe, nprot, cuf, finalidade, saida, id_nf_ref, dhemi "
              "FROM notas_fiscais WHERE cstat = 'CONTINGENCIA'");

    if (q.exec()) {
        while (q.next()) {
            NotaFiscalDTO nota;
            nota.cstat      = q.value("cstat").toString();
            nota.chNfe      = q.value("chnfe").toString();
            nota.xmlPath    = q.value("xml_path").toString();
            nota.modelo     = q.value("modelo").toString();
            nota.nnf        = q.value("nnf").toLongLong();
            nota.nProt      = q.value("nprot").toString();
            nota.finalidade = q.value("finalidade").toString();
            nota.dhEmi      = q.value("dhemi").toString();
            lista.append(nota);
        }
    } else {
        qDebug() << "ContingenciaRepository::buscarPendentes:" << q.lastError().text();
    }

    return lista;
}

QList<NotaFiscalDTO> ContingenciaRepository::reservarPendentes(int limite, const QString &computador)
{
    QList<NotaFiscalDTO> lista;

    if (limite <= 0)
        return lista;
    const bool postgres = m_db.driverName() == "QPSQL";
    const bool sqlite = m_db.driverName() == "QSQLITE";

    qDebug() << "Driver:" << m_db.driverName();
    // Inicia transação com estratégia específica para cada banco.
    QSqlQuery controle(m_db);

    if (sqlite) {
        if (!controle.exec("BEGIN IMMEDIATE")) {
            qDebug() << "Erro ao iniciar transação SQLite:"
                     << controle.lastError().text();
            return lista;
        }
    } else {
        if (!m_db.transaction()) {
            qDebug() << "Erro ao iniciar transação:"
                     << m_db.lastError().text();
            return lista;
        }
    }

    auto rollback = [&]() {
        QSqlQuery q(m_db);

        if (sqlite)
            q.exec("ROLLBACK");
        else
            m_db.rollback();
    };

    // Seleciona notas disponíveis.
    QString sql =
        "SELECT cstat, nnf, serie, modelo, tp_amb, xml_path, valor_total, "
        "id_venda, cnpjemit, chnfe, nprot, cuf, finalidade, saida, "
        "id_nf_ref, dhemi "
        "FROM notas_fiscais "
        "WHERE cstat = 'CONTINGENCIA' "
        "AND (processando_ate IS NULL OR ";

    if (sqlite)
        sql += "datetime(processando_ate) < datetime('now')) ";
    else
        sql += "processando_ate < CURRENT_TIMESTAMP) ";

    sql += "ORDER BY dhemi LIMIT :limite";

    if (!sqlite)
        sql += " FOR UPDATE SKIP LOCKED";

    QSqlQuery select(m_db);
    select.prepare(sql);
    select.bindValue(":limite", limite);

    if (!select.exec()) {
        qDebug() << "Erro ao buscar notas pendentes:"
                 << select.lastError().text();
        rollback();
        return lista;
    }

    // Carrega as notas em memória antes de executar os UPDATEs.
    while (select.next()) {
        NotaFiscalDTO nota;

        nota.cstat      = select.value("cstat").toString();
        nota.chNfe      = select.value("chnfe").toString();
        nota.xmlPath    = select.value("xml_path").toString();
        nota.modelo     = select.value("modelo").toString();
        nota.nnf        = select.value("nnf").toLongLong();
        nota.nProt      = select.value("nprot").toString();
        nota.finalidade = select.value("finalidade").toString();
        nota.dhEmi      = select.value("dhemi").toString();

        lista.append(nota);
    }

    select.finish();

    // Reserva individualmente cada nota.
    for (NotaFiscalDTO &nota : lista) {

        nota.tokenReserva =
            QUuid::createUuid().toString(QUuid::WithoutBraces);

        QSqlQuery update(m_db);

        QString updateSql =
            "UPDATE notas_fiscais SET "
            "token_reserva = ?, "
            "processando_por = ?, "
            "processando_ate = ? "
            "WHERE chnfe = ? "
            "AND cstat = 'CONTINGENCIA' "
            "AND (processando_ate IS NULL OR ";

        if (sqlite) {
            updateSql += "datetime(processando_ate) < datetime('now'))";
        } else if (postgres) {
            updateSql += "processando_ate < CURRENT_TIMESTAMP)";
        } else {
            qDebug() << "Driver de banco não suportado:"
                     << m_db.driverName();
            rollback();
            return {};
        }

        if (!update.prepare(updateSql)) {
            qDebug() << "Erro ao preparar UPDATE:"
                     << update.lastError().text();
            rollback();
            return {};
        }

        QDateTime expiracao =
            QDateTime::currentDateTimeUtc().addSecs(300);

        // A ordem dos valores deve corresponder à ordem dos ? no SQL.
        update.addBindValue(nota.tokenReserva);  // 1: token_reserva
        update.addBindValue(computador);         // 2: processando_por

        if (sqlite) {
            update.addBindValue(expiracao.toString(Qt::ISODate));
        } else {
            update.addBindValue(expiracao);
        }

        update.addBindValue(nota.chNfe);         // 4: chnfe

        if (!update.exec() || update.numRowsAffected() != 1) {
            qDebug() << "Erro ao reservar nota:" << nota.chNfe
                     << update.lastError().text();

            rollback();
            return {};
        }
    }

    // Confirma todas as reservas de uma vez.
    bool confirmado;

    if (sqlite) {
        QSqlQuery commit(m_db);
        confirmado = commit.exec("COMMIT");
    } else {
        confirmado = m_db.commit();
    }

    if (!confirmado) {
        qDebug() << "Erro ao confirmar reservas:"
                 << m_db.lastError().text();

        rollback();
        return {};
    }

    qDebug() << "ContingenciaRepository:"
             << lista.size() << "nota(s) reservadas por" << computador;

    return lista;
}

bool ContingenciaRepository::atualizar(const QString &chNfe, const QString &tokenReserva, const QString &cstat,
               const QString &nProt)
{
    qDebug() << "Atualizando nota:" << chNfe;
    qDebug() << "Token recebido:" << tokenReserva;
    qDebug() << "Novo cStat:" << cstat;
    QSqlQuery q(m_db);
    q.prepare(
        "UPDATE notas_fiscais SET "
        "cstat = :cstat, "
        "nprot = :nprot, "
        "atualizado_em = :atualizadoem, "
        "token_reserva = NULL, "
        "processando_por = NULL, "
        "processando_ate = NULL "
        "WHERE chnfe = :chnfe "
        "AND token_reserva = :token"
        );

    q.bindValue(":cstat", cstat);
    q.bindValue(":nprot", nProt);
    q.bindValue(":atualizadoem", DataUtil::getDataAgoraUS());
    q.bindValue(":chnfe", chNfe);
    q.bindValue(":token", tokenReserva);

    bool ok = q.exec();

    if (!ok) {
        qDebug() << "Erro ao atualizar contingência:"
                 << q.lastError().text();
    } else if (q.numRowsAffected() != 1) {
        qDebug() << "Reserva perdida ou expirada para:" << chNfe;
        return false;
    }

    return ok;
}
