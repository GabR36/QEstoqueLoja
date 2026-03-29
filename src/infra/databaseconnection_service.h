#ifndef DATABASECONNECTION_SERVICE_H
#define DATABASECONNECTION_SERVICE_H

#include <QObject>
#include <QSqlDatabase>

class DatabaseConnection_service : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseConnection_service(QObject *parent = nullptr);
    static bool init();
    static bool open();
    void close();
    static QSqlDatabase db();
    static void setDatabase(QSqlDatabase database);
private:
     static bool initialized;

signals:
};

#endif // DATABASECONNECTION_SERVICE_H
