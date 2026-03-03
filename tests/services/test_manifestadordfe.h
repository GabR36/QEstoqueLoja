#ifndef TEST_MANIFESTADORDFE_H
#define TEST_MANIFESTADORDFE_H

#include <QObject>
#include "../util/manifestadordfe.h"
#include <QtTest>


class test_manifestadordfe : public QObject
{
    Q_OBJECT
public:
    explicit test_manifestadordfe(QObject *parent = nullptr);
public slots:
private slots:
    void verificar_resumosalvo_retornoForcado();
    void verificar_NotaFiscalEntradaSalvo_retornoForcado();
    void cleanupTestCase();

private:
    QSqlDatabase db;
signals:
};

#endif // TEST_MANIFESTADORDFE_H
