#include "vendas_service.h"

Vendas_service::Vendas_service(QObject *parent)
    : QObject{parent}
{}


qlonglong Vendas_service::getQuantidadeComprasCliente(qlonglong idcli){
    return vendasRepo.getQuantidadeComprasCliente(idcli);
}

double Vendas_service::getValorTotalVendasPrazoCliente(qlonglong idcliente){
    return vendasRepo.getValorTotalVendasPrazoCliente(idcliente);
}
