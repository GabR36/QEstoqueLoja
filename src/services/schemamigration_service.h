#ifndef SCHEMAMIGRATION_SERVICE_H
#define SCHEMAMIGRATION_SERVICE_H

#include <QObject>
#include <QSqlDatabase>
#include "config_service.h"

enum class SchemaErro {
    Nenhum,
    CampoVazio,
    FalhaConexao,
    ErroSQL,
    ErroMigracao
};

class SchemaMigration_service : public QObject
{
    Q_OBJECT

public:

    struct Resultado {
        bool ok;
        SchemaErro status;
        QString message;
        int lastVersion;
    };
    explicit SchemaMigration_service(QObject *parent = nullptr, int dbLastVersion = 0);

    int dbSchemaLastVersion;

    int dbSchemaVersion;
    ConfigDTO configDTO;

    QSqlDatabase db;
    // void update();

    SchemaMigration_service::Resultado update();

signals:
    void dbVersao6();
    void dbVersao7();
};

#endif // SCHEMAMANAGER_H
