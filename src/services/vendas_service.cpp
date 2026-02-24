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

QSqlQueryModel* Vendas_service::listarVendas(){
    return vendasRepo.listarVendas();
}

QPair<QDate, QDate> Vendas_service::getMinMaxData(){
    return vendasRepo.getMinMaxData();
}

VendasDTO Vendas_service::getVenda(qlonglong id){
    return vendasRepo.getVenda(id);
}

