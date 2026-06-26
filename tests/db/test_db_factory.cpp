#include "test_db_factory.h"
#include <QDebug>
#include <QDir>
#include "infra/databaseconnection_service.h"
#include "services/schemamigration_service.h"
#include <QProcess>
#include <QSqlQuery>
#include <QUuid>

QString TestDbFactory::currentConnectionName;
QString TestDbFactory::currentDatabaseName;

QSqlDatabase TestDbFactory::create()
{
    QString connName = "test_conn_1";

    QString path = QDir::tempPath() + "/qestoque_test_" + connName + ".db";

    // cria conexão nomeada
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
    db.setDatabaseName(path);

    if (!db.open()) {
        qFatal("Erro ao abrir banco de teste");
    }
    qDebug() << "Rodando Teste usando SQLITE.";
    qDebug() << "Connection name:" << db.connectionName();
    qDebug() << "Is open:" << db.isOpen();
    qDebug() << "Path:" << path;

    // registra essa conexão no seu service
    DatabaseConnection_service::setDatabase(db);

    // roda migration
    SchemaMigration_service schema(nullptr, 12);
    auto result = schema.update();

    if (!result.ok) {
        qFatal("Falha ao rodar schema migration no banco de teste");
    }

    return db;
}

QSqlDatabase TestDbFactory::createPostgres()
{
    qDebug() << "Rodando Teste usando Postgre.";

    QString connName = "test_pg_conn";
    currentConnectionName = connName;

    QString dbName =
        "qestoque_test_" +
        QUuid::createUuid()
            .toString()
            .remove("-")
            .remove("{")
            .remove("}");
    currentDatabaseName = dbName;
    // conexão administrativa
    QString adminConnName = "postgres_admin";

    QSqlDatabase admin =
        QSqlDatabase::addDatabase("QPSQL", adminConnName);

    // admin.setHostName("localhost");
    // admin.setPort(5432);
    // admin.setDatabaseName("postgres");
    // admin.setUserName("qestoque-user");
    // admin.setPassword("1234");

    admin.setHostName(
        qEnvironmentVariable("TEST_DB_HOST", "localhost")
        );

    admin.setPort(
        qEnvironmentVariableIntValue("TEST_DB_PORT")
        );

    admin.setDatabaseName("postgres");

    admin.setUserName(
        qEnvironmentVariable("TEST_DB_USER", "postgres")
        );

    admin.setPassword(
        qEnvironmentVariable("TEST_DB_PASSWORD")
        );


    if (!admin.open()) {
        qFatal("Erro ao conectar no postgres admin: %s",
               qPrintable(admin.lastError().text()));
    }


    // cria banco temporário
    QSqlQuery create(admin);

    if (!create.exec(
            QString("CREATE DATABASE %1")
                .arg(dbName)
            )) {

        qFatal("Erro criando banco teste postgres: %s",
               qPrintable(create.lastError().text()));
    }


    admin.close();
    admin = QSqlDatabase();

    QSqlDatabase::removeDatabase(adminConnName);



    // conexão no banco criado
    QSqlDatabase db =
        QSqlDatabase::addDatabase("QPSQL", connName);


    db.setHostName("localhost");
    db.setPort(5432);
    db.setDatabaseName(dbName);
    db.setUserName("qestoque-user");
    db.setPassword("1234");


    if (!db.open()) {
        qFatal("Erro abrindo banco teste postgres: %s",
               qPrintable(db.lastError().text()));
    }


    qDebug() << "Banco postgres teste criado:"
             << dbName;


    DatabaseConnection_service::setDatabase(db);


    SchemaMigration_service schema(nullptr, 12);

    auto result = schema.update();


    if (!result.ok) {
        qFatal("Falha migration postgres teste");
    }


    return db;
}

void TestDbFactory::removerBDAtual(){
    if (QSqlDatabase::contains(currentConnectionName)) {
        QSqlDatabase db = QSqlDatabase::database(currentConnectionName);
        if (db.isOpen()) {
            db.close();
        }
        QSqlDatabase::removeDatabase(currentConnectionName);
        qDebug() << "Conexão removida:" << currentConnectionName;
    }

    if (!currentDatabaseName.isEmpty()) {
        QString adminConnName = "postgres_admin_drop";

        QSqlDatabase admin = QSqlDatabase::addDatabase("QPSQL", adminConnName);
        admin.setHostName("localhost");
        admin.setPort(5432);
        admin.setDatabaseName("postgres");
        admin.setUserName("qestoque-user");
        admin.setPassword("1234");

        if (!admin.open()) {
            qWarning() << "Erro ao conectar para dropar banco:" << admin.lastError().text();
            return;
        }

        QSqlQuery terminate(admin);
        QString terminateQuery = QString(
                                     "SELECT pg_terminate_backend(pg_stat_activity.pid) "
                                     "FROM pg_stat_activity "
                                     "WHERE pg_stat_activity.datname = '%1' "
                                     "AND pid <> pg_backend_pid()"
                                     ).arg(currentDatabaseName);

        if (!terminate.exec(terminateQuery)) {
            qWarning() << "Erro ao terminar conexões:" << terminate.lastError().text();
        }

        // Droppa o banco
        QSqlQuery drop(admin);
        if (!drop.exec(QString("DROP DATABASE IF EXISTS %1").arg(currentDatabaseName))) {
            qWarning() << "Erro ao dropar banco" << currentDatabaseName << ":" << drop.lastError().text();
        } else {
            qDebug() << "Banco PostgreSQL removido:" << currentDatabaseName;
        }

        admin.close();
        QSqlDatabase::removeDatabase(adminConnName);

        currentDatabaseName = "";
    }

    currentConnectionName = "";
}
