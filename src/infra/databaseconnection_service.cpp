#include "databaseconnection_service.h"
#include "apppath_service.h"
#include <QSqlError>
#include <QDebug>

bool DatabaseConnection_service::initialized = false;

static QSqlDatabase externalDb;
static bool hasExternalDb = false;
static DatabaseConfig config = {
    "QPSQL",
    "localhost",
    5432,
    "qestoque-script",
    "qestoque-user",
    "1234"
};
// static DatabaseConfig config = {
//     "QSQLITE",
//     "",
//     0,
//     "",
//     "",
//     ""
// };


DatabaseConnection_service::DatabaseConnection_service(QObject *parent)
    : QObject{parent}
{}

void DatabaseConnection_service::setDatabase(QSqlDatabase database)
{
    externalDb = database;
    hasExternalDb = true;
    initialized = true;
}


bool DatabaseConnection_service::init()
{
    // qDebug() << QSqlDatabase::drivers();
    if (hasExternalDb) return true;
    if (initialized) return true;

    try {

        if (!QSqlDatabase::contains("qt_sql_default_connection")) {

            QSqlDatabase db =
                QSqlDatabase::addDatabase(config.driver);

            if(config.driver == "QSQLITE")
            {
                db.setDatabaseName(AppPath_service::databasePath());
            }
            else if(config.driver == "QPSQL")
            {
                db.setHostName(config.host);
                db.setPort(config.port);
                db.setDatabaseName(config.database);
                db.setUserName(config.user);
                db.setPassword(config.password);
            }
        }

        initialized = true;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool DatabaseConnection_service::isPostgres()
{
    QSqlDatabase db = hasExternalDb
                          ? externalDb
                          : QSqlDatabase::database();
    return db.driverName().contains("QPSQL");
}

bool DatabaseConnection_service::open()
{
    init();

    QSqlDatabase db = hasExternalDb
                          ? externalDb
                          : QSqlDatabase::database();

    // qDebug() << "Driver:" << db.driverName();
    // qDebug() << "Host:" << db.hostName();
    // qDebug() << "Database:" << db.databaseName();
    // qDebug() << "User:" << db.userName();
    // qDebug() << "IsValid:" << db.isValid();
    // qDebug() << "IsOpen:" << db.isOpen();

    if (!db.isOpen()) {

        qDebug() << "Tentando abrir conexão...";

        if (!db.open()) {

            qDebug() << "ERRO AO ABRIR";
            qDebug() << db.lastError().text();
            qDebug() << db.lastError().driverText();
            qDebug() << db.lastError().databaseText();

            return false;
        }
    }

    qDebug() << "Conectado com sucesso";

    return true;
}

void DatabaseConnection_service::close()
{
    QSqlDatabase db = hasExternalDb
                          ? externalDb
                          : QSqlDatabase::database();

    if (db.isOpen()) {
        db.close();
    }
}

QSqlDatabase DatabaseConnection_service::db()
{
    init();

    return hasExternalDb
               ? externalDb
               : QSqlDatabase::database();
}


QSqlDatabase DatabaseConnection_service::createThreadConnection(const QString &name)
{
    if (QSqlDatabase::contains(name))
        return QSqlDatabase::database(name);


    QSqlDatabase db =
        QSqlDatabase::addDatabase(config.driver, name);


    if(config.driver == "QSQLITE")
    {
        db.setDatabaseName(AppPath_service::databasePath());
    }
    else if(config.driver == "QPSQL")
    {
        db.setHostName(config.host);
        db.setPort(config.port);
        db.setDatabaseName(config.database);
        db.setUserName(config.user);
        db.setPassword(config.password);
    }


    return db;
}
