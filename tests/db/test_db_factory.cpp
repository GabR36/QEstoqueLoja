#include "test_db_factory.h"
#include <QDebug>
#include <QDir>
#include "infra/databaseconnection_service.h"
#include "services/schemamigration_service.h"

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

    qDebug() << "Connection name:" << db.connectionName();
    qDebug() << "Is open:" << db.isOpen();
    qDebug() << "Path:" << path;

    // registra essa conexão no seu service
    DatabaseConnection_service::setDatabase(db);

    // roda migration
    SchemaMigration_service schema(nullptr, 7);
    auto result = schema.update();

    if (!result.ok) {
        qFatal("Falha ao rodar schema migration no banco de teste");
    }

    return db;
}
