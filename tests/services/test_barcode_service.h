#ifndef TEST_BARCODE_SERVICE_H
#define TEST_BARCODE_SERVICE_H

#include <QObject>
#include <QtTest>
#include "../services/barcode_service.h"

class test_barcode_service : public QObject
{
    Q_OBJECT
public:
    explicit test_barcode_service(QObject *parent = nullptr);

signals:
private slots:
    void codigoValido_deveGerarImagem();
    void codigoVazio_deveFalhar();
};

#endif // TEST_BARCODE_SERVICE_H
