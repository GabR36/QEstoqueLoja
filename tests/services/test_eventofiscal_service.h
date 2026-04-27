#ifndef TEST_EVENTOFISCAL_SERVICE_H
#define TEST_EVENTOFISCAL_SERVICE_H

#include <QObject>
#include <QtTest>
#include "../services/eventofiscal_service.h"

class test_eventofiscal_service : public QObject
{
    Q_OBJECT
public:
    explicit test_eventofiscal_service(QObject *parent = nullptr);
private slots:
    void enviarCienciaOp_sucesso();
    void enviarCienciaOp_deveFalharQuandoSefazRejeita();
    void init();
private:
    QSqlDatabase db;

signals:
};

#endif // TEST_EVENTOFISCAL_SERVICE_H
