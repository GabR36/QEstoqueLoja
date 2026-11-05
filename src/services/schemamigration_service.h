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
    ErroMigracao,
    ProgramaDesatualizado
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
    Config_service confServ;
    ConfigDTO configDTO;

    QSqlDatabase db;
    // void update();

    SchemaMigration_service::Resultado update();

    SchemaMigration_service::Resultado bancoComVersaoSuperior();
signals:
    void dbVersao6();
    void dbVersao7();
    void dbVersao9();
    void dbVersao10();
    void dbVersao11();
    void dbVersao13();
    void dbVersao14();
private slots:
    bool executarArquivoSql(const QString &caminho);
private:
    int getSchemaVersion();
    bool setSchemaVersion(int version);
};

#endif // SCHEMAMANAGER_H
