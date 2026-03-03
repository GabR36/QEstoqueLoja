#ifndef APPPATH_SERVICE_H
#define APPPATH_SERVICE_H

#include <QObject>

class AppPath_service : public QObject
{
    Q_OBJECT
public:
    explicit AppPath_service(QObject *parent = nullptr);

    static QString appDataPath();
    static QString databasePath();
    static QString xmlPath();
    static QString schemaPath();
    static QString imagesPath();
signals:
};

#endif // APPPATH_SERVICE_H
