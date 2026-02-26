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

void Vendas_service::listarVendas(QSqlQueryModel *model){
    return vendasRepo.listarVendas(model);
}

QPair<QDate, QDate> Vendas_service::getMinMaxData(){
    return vendasRepo.getMinMaxData();
}

VendasDTO Vendas_service::getVenda(qlonglong id){
    return vendasRepo.getVenda(id);
}

void Vendas_service::listarVendasDeAteFormaPag(QSqlQueryModel *model,
                                               QString de,
                                               QString ate,
                                               VendasUtil::VendasFormaPagamento formaPag){
    return vendasRepo.listarVendasDeAteFormaPagamento(model, de, ate, formaPag);
}

ResumoVendasDTO Vendas_service::calcularResumo(
    const QString& dataDe,
    const QString& dataAte,
    bool somentePrazo,
    qlonglong idCliente)
{
    return vendasRepo.calcularResumo(dataDe, dataAte, somentePrazo, idCliente);
}
