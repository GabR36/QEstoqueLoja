#include "vendas_service.h"
#include <QDebug>

Vendas_service::Vendas_service(QObject *parent)
    : QObject{parent}
{
    confDTO = confServ.carregarTudo();
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
                                               VendasUtil::VendasFormaPagamento formaPag,
                                               qlonglong idcliente){
    return vendasRepo.listarVendasDeAteFormaPagamento(model, de, ate, formaPag, idcliente);
}

ResumoVendasDTO Vendas_service::calcularResumo(
    const QString& dataDe,
    const QString& dataAte,
    bool somentePrazo,
    qlonglong idCliente)
{
    ResumoVendasDTO resumo = vendasRepo.calcularResumo(dataDe, dataAte, somentePrazo, idCliente);
    double porcent = confDTO.porcentLucroFinanceiro;
    double custo = resumo.total / (1.0 + (porcent / 100));
    resumo.lucro = resumo.total - custo;
    return resumo;
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

Vendas_service::Resultado Vendas_service::updateNewTotalTrocoValorFinal(double total, double troco, double valorFinal, qlonglong id){
    if(!vendasRepo.updateNewTotalTrocoValorFinal(total, troco, valorFinal, id)){
        return {false, VendasErro::UpdateFalhou,  "Erro ao atualizar valores"};
    }else{
        return {true, VendasErro::Nenhum, "Update OK"};
    }
}


Vendas_service::Resultado Vendas_service::devolverProdutoRegraNegocio(qlonglong idProdVend,
                                                                                  qlonglong idVenda){
    ProdutoVendidoDTO produtoVendido = prodVendaServ.getProdutoVendido(idProdVend);

    qDebug() << "id_produto: " << produtoVendido.idProduto;
    qDebug() << "qntd: " << produtoVendido.quantidade;
    qDebug() << "preco_vend: " << produtoVendido.precoVendido;

    // deletar registro do produto devolvido
    auto result = prodVendaServ.deletarProdutoVendido(idProdVend);
    if(!result.ok){
        return {false, VendasErro::DeleteFalhou, result.msg};
    }

    auto result1 = prodServ.updateAumentarQuantidadeProduto(produtoVendido.idProduto, produtoVendido.quantidade);
    if(!result1.ok){
        return {false, VendasErro::UpdateFalhou, result1.msg};
    }

    VendasDTO venda = getVenda(idVenda);
    qDebug() << "total: " + QString::number(venda.total);
    qDebug() << "taxa: " + QString::number(venda.taxa);
    qDebug() << "desconto: " + QString::number(venda.desconto);
    qDebug() << "recebido: " + QString::number(venda.valorRecebido);

    // mudar o registro da venda para retirar o valor do produto devolvido

    double taxa = 1 + venda.taxa/100;
    double totalSub = produtoVendido.quantidade * produtoVendido.precoVendido;
    double totalNovo = venda.total - totalSub;
    double valorFinalNovo = (totalNovo - venda.desconto)*taxa;
    double trocoNovo = venda.valorRecebido - valorFinalNovo;

    auto result2 = updateNewTotalTrocoValorFinal(totalNovo, trocoNovo, valorFinalNovo, idVenda);
    if(!result2.ok){
        return {false, VendasErro::UpdateFalhou, result2.msg};
    }

    return {true, VendasErro::Nenhum, "Produto Devolvido com sucesso"};
}

void Vendas_service::listarVendasCliente(QSqlQueryModel *model, qlonglong idcliente)
{
    return vendasRepo.listarVendasCliente(model, idcliente);
}


Vendas_service::ResultadoInsercaoRN Vendas_service::inserirVendaRegraDeNegocio(VendasDTO venda,
                                                                     QList<ProdutoVendidoDTO> listaProds){

    //  ser menor ou igual ao valor final
    bool menorQueTotal = venda.desconto <= venda.total;

    if (!menorQueTotal){
        return {false, VendasErro::QuebraDeRegra, "Valor do desconto maior que o total.", -1};
    }

    if(venda.formaPagamento == "Prazo" && venda.idCliente <= 1){
        return {false, VendasErro::QuebraDeRegra, "Cliente não especificado para venda a prazo.", -1};
    }

    // testar se o recebido é maior ou igual ao valor final
    if (!venda.valorRecebido >= venda.valorFinal){
        // caso não seja maior ou igual que o total avalie como erro.
        return {false, VendasErro::QuebraDeRegra, "Valor Recebido deve ser maior que o valor final.", -1};
    }
    qlonglong idVenda = vendasRepo.inserir(venda);
    if(idVenda <= 0){
        return{false, VendasErro::InsercaoInvalida, "Erro ao inserir venda no banco de dados.", -1};
    }

    for(int i = 0; i < listaProds.size(); i++){
        ProdutoVendidoDTO prod = listaProds[i];
        prod.idVenda = idVenda;
        auto result = prodVendaServ.inserir(prod);
        if(!result.ok){
            return {false, VendasErro::InsercaoInvalida, result.msg, -1};
        }
        auto result2 = prodServ.updateDiminuirQuantidadeProduto(prod.idProduto, prod.quantidade);
        if(!result2.ok){
            return {false, VendasErro::UpdateFalhou, result2.msg, -1};
        }
    }

    return{true, VendasErro::Nenhum, "Sucesso ao inserir venda.", idVenda};

}

Vendas_service::Resultado Vendas_service::atualizarEstaPago(qlonglong idVenda, bool estaPago){
    if(!vendasRepo.atualizarEstaPago(idVenda, estaPago ? 1 : 0)){
        return {false, VendasErro::UpdateFalhou, "Erro ao atualizar esta_pago da venda."};
    }
    return {true, VendasErro::Nenhum, "esta_pago atualizado com sucesso."};
}
