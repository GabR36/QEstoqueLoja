// #ifndef SCHEMAMANAGER_H
// #define SCHEMAMANAGER_H

// #include <QObject>
// #include <QSqlDatabase>

// enum class SchemaErro {
//     Nenhum,
//     CampoVazio,
//     FalhaConexao,
//     ErroSQL,
//     ErroMigracao
// };

// class SchemaManager : public QObject
// {
//     Q_OBJECT

// public:

//     struct Resultado {
//         bool ok;
//         SchemaErro status;
//         QString message;
//         int lastVersion;
//     };
//     explicit SchemaManager(QObject *parent = nullptr, int dbLastVersion = 0);

//     int dbSchemaLastVersion;

//     int dbSchemaVersion;

//     QSqlDatabase db;
//     // void update();

//     SchemaManager::Resultado update();

// signals:
//     void dbVersao6();
//     void dbVersao7();
// };

// #endif // SCHEMAMANAGER_H
