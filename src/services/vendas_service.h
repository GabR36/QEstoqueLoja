#ifndef VENDAS_SERVICE_H
#define VENDAS_SERVICE_H

#include <QObject>
#include "../repository/vendas_repository.h"

enum class VendasErro {
    Nenhum,
    CampoVazio,
    Banco,
    InsercaoInvalida,
    DeleteFalhou,
    QuebraDeRegra
};

class Vendas_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        VendasErro erro = VendasErro::Nenhum;
        QString msg;
    };
    explicit Vendas_service(QObject *parent = nullptr);
    qlonglong getQuantidadeComprasCliente(qlonglong idcli);
    double getValorTotalVendasPrazoCliente(qlonglong idcliente);
private:
    Vendas_repository vendasRepo;

signals:
};

#endif // VENDAS_SERVICE_H
