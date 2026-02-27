#include "vendas_service.h"


Vendas_service::Vendas_service(QObject *parent)
    : QObject{parent}
{

}


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

Vendas_service::Resultado Vendas_service::deletarVenda(qlonglong id){
    if(!vendasRepo.deletarVenda(id)){
        return {false, VendasErro::DeleteFalhou, "Erro ao deletar venda."};
    }else{
        return {true, VendasErro::Nenhum, "Sucesso ao deletar venda"};
    }
}

Vendas_service::Resultado
Vendas_service::deletarVendaRegraNegocio(qlonglong idVenda, bool cancelarNf)
{
        if(cancelarNf){
            qlonglong idNf = notaServ.getIdFromIdVenda(idVenda);
            if(idNf != -1){
                auto resultado = eventoServ.enviarCancelamento(idNf);
                if(!resultado.ok)
                    return {false, VendasErro::EventoFiscal, resultado.msg};
            }
        }

        QList<ProdutoVendidoDTO> listaProdutos =
            prodVendaServ.getProdutosVendidos(idVenda);

        for(const auto &prod : listaProdutos){
            auto result2 = prodServ.updateAumentarQuantidadeProduto(prod.idProduto, prod.quantidade);
            if(!result2.ok){
                return {false, VendasErro::Produto, result2.msg};
            }
        }

        auto result3 = prodVendaServ.deletarProdutosVendidosPorIdVenda(idVenda);
        if(!result3.ok){
            return {false, VendasErro::ProdutoVendido, result3.msg};
        }

        auto result4 = entradaServ.deletarPorIdVenda(idVenda);
        if(!result4.ok){
            return {false, VendasErro::EntradasVendas, result4.msg};
        }
        auto result5 = deletarVenda(idVenda);
        if(!result5.ok){
            return {false, VendasErro::DeleteFalhou, result5.msg};
        }
        return {true, VendasErro::Nenhum, "Venda deletada com sucesso"};
}

bool Vendas_service::vendaPossuiNota(qlonglong idVenda)
{
    return notaServ.getIdFromIdVenda(idVenda) != -1;
}
