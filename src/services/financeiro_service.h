#ifndef FINANCEIRO_SERVICE_H
#define FINANCEIRO_SERVICE_H

#include <QObject>
#include <QMap>
#include <QDate>
#include "entradasvendas_service.h"
#include "vendas_service.h"
#include "../repository/relatorios_repository.h"


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

    // lucro estimado = lucro bruto dos itens vendidos, ja descontando desconto
    // dado e taxa de cartao (credito/debito), proporcional ao valor recebido em
    // caixa (relevante para vendas a prazo pagas em varias entradas).
    double getLucroEstimadoVenda(qlonglong idVenda);
    QMap<QString, double> getLucroEstimadoPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup);
private:
    Vendas_service vendaServ;
    EntradasVendas_service entradasServ;
    Relatorios_repository relatoriosRepo;

signals:
};

#endif // FINANCEIRO_SERVICE_H
