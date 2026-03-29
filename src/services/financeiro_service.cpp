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
