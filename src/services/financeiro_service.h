#ifndef FINANCEIRO_SERVICE_H
#define FINANCEIRO_SERVICE_H

#include <QObject>
#include "entradasvendas_service.h"
#include "vendas_service.h"


enum class FinanceiroErro {
    Nenhum,
    CampoVazio,
    Banco,
    InsercaoInvalida,
    DeleteFalhou,
    QuebraDeRegra
};


class Financeiro_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        FinanceiroErro erro = FinanceiroErro::Nenhum;
        QString msg;
    };
    explicit Financeiro_service(QObject *parent = nullptr);
    double getValorTotalDevidoFromCliente(qlonglong idcliente);
private:
    Vendas_service vendaServ;
    EntradasVendas_service entradasServ;

signals:
};

#endif // FINANCEIRO_SERVICE_H
