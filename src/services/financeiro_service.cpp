#include "financeiro_service.h"

Financeiro_service::Financeiro_service(QObject *parent)
    : QObject{parent}
{}


double Financeiro_service::getValorTotalDevidoFromCliente(qlonglong idcliente){
    double valorTotalVendas = vendaServ.getValorTotalVendasPrazoCliente(idcliente);
    double valorTotalEntradas = entradasServ.getValorTotalEntradasFromClientes(idcliente);
    double valorDevido = valorTotalVendas - valorTotalEntradas;
    return valorDevido;
}

double Financeiro_service::getLucroEstimadoVenda(qlonglong idVenda)
{
    return relatoriosRepo.buscarLucroVenda(idVenda);
}

QMap<QString, double> Financeiro_service::getLucroEstimadoPeriodo(
    const QDate &inicio, const QDate &fim, Agrupamento agrup)
{
    return relatoriosRepo.buscarLucroPeriodo(inicio, fim, agrup);
}
