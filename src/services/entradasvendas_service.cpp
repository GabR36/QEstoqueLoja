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

QList<EntradaVendaDTO> EntradasVendas_service::getEntradasFromVenda(qlonglong idvenda){
    return entradaRepo.getEntradasFromVenda(idvenda);
}

void EntradasVendas_service::listarEntradasVenda(QSqlQueryModel *model, qlonglong idvenda){
    entradaRepo.listarEntradasVenda(model, idvenda);
}

EntradasVendas_service::Resultado EntradasVendas_service::deletarEntradaPorId(qlonglong id){
    if(!entradaRepo.deletarEntradaPorId(id)){
        return{false, EntradasVendasErro::DeleteFalhou, "Erro ao deletar entrada"};
    }
    return{true, EntradasVendasErro::Nenhum, "Entrada deletada com sucesso"};
}

EntradasVendas_service::Resultado EntradasVendas_service::deletarPorIdVenda(qlonglong idvenda){
    if(!entradaRepo.deletarPorIdVenda(idvenda)){
        return{false, EntradasVendasErro::DeleteFalhou, "Erro ao deletar recebimentos"};
    }else{
        return{true, EntradasVendasErro::Nenhum, "Sucesso ao deletar entradas recebimentos"};
    }
}
