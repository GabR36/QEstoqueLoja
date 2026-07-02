#ifndef EXPORT_SERVICE_H
#define EXPORT_SERVICE_H

#include <QObject>

class Export_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        QString msg;
    };
    explicit Export_service(QObject *parent = nullptr);

    Export_service::Resultado exportarSqliteDB(QString pathDestino);
signals:
};

#endif // EXPORT_SERVICE_H
