#include "entradasvendas_service.h"
#include <QDateTime>

EntradasVendas_service::EntradasVendas_service(QObject *parent)
    : QObject{parent}
{}

QDateTime EntradasVendas_service::getDataUltimoPagamentoFromCliente(qlonglong idcliente){
    QDateTime data = entradaRepo.getDataUltimoPagamentoFromCliente(idcliente);
    return data;
}

double EntradasVendas_service::getValorUltimoPagamentoFromCliente(qlonglong idcliente){
    return entradaRepo.getValorUltimoPagamentoFromCliente(idcliente);
}

double EntradasVendas_service::getValorTotalEntradasFromClientes(qlonglong idcliente){
    return entradaRepo.getValorTotalEntradasFromClientes(idcliente);
}
