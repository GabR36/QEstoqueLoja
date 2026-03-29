#include "apppath_service.h"
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

AppPath_service::AppPath_service(QObject *parent)
    : QObject{parent}
{}

QString AppPath_service::appDataPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return path;
}

QString AppPath_service::databasePath()
{
    QString path = appDataPath() + "/estoque.db";
    return path;
}

QString AppPath_service::xmlPath()
{
    QString path = appDataPath() + "/xmlNf";
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return path;
}

QString AppPath_service::schemaPath()
{
    QString path = appDataPath() + "/recursos/NFeSchemas";
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return path;
}

QString AppPath_service::imagesPath()
{
    AppPath_service pathService;
    QString path = pathService.appDataPath() + "/imagens";
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return path;
}
