#ifndef SCHEMAMANAGER_H
#define SCHEMAMANAGER_H

#include <QObject>
#include <QSqlDatabase>

class SchemaManager : public QObject
{
    Q_OBJECT
public:
    explicit SchemaManager(QObject *parent = nullptr, int dbLastVersion = 0);

    void update();

    int dbSchemaLastVersion;

    int dbSchemaVersion;

    QSqlDatabase db;
signals:
    void dbVersao6();
    void dbVersao7();
};

#endif // SCHEMAMANAGER_H
