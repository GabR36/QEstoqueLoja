#ifndef DATABASECONNECTION_SERVICE_H
#define DATABASECONNECTION_SERVICE_H

#include <QObject>
#include <QSqlDatabase>
#include "../dto/Config_dto.h".h"

struct DatabaseConfig
{
    QString driver;
    QString host;
    int port;
    QString database;
    QString user;
    QString password;
};


class DatabaseConnection_service
{

public:
    static bool init();
    static bool open();
    void close();
    static QSqlDatabase db();
    static void setDatabase(QSqlDatabase database);
    static bool isPostgres();
    static QSqlDatabase createThreadConnection(const QString &name);
    static void changeDatabase(ConfigDTO configDto);
private:
     static bool initialized;

signals:
};

#endif // DATABASECONNECTION_SERVICE_H
