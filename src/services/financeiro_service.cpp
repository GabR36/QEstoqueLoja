#include "financeiro_service.h"
#include "../dto/Vendas_dto.h"
#include "../dto/ProdutoVendido_dto.h"
#include "Produto_service.h"

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

double Financeiro_service::getLucroEstimadoVendaDto(VendasDTO venda, QList<ProdutoVendidoDTO> produtos)
{
    double lucroTotal = 0.0;
    ProdutoDTO prod;

    QList<ProdutoDTO> prods;
    prods = prodServ.getListaProdutosViaListaVendidos(produtos);

    // Calcula o lucro individual de cada produto
    for (const ProdutoVendidoDTO &produto : produtos) {
        for (const ProdutoDTO &prod1 : prods) {
            if (prod1.id == produto.id) {
                prod = prod1;
                break;
            }
        }

        double custoReal = produto.precoVendido *
                           (1.0 - prod.percentLucro / 100.0);

        double lucroProduto = produto.quantidade *
                              (produto.precoVendido - custoReal);

        lucroTotal += lucroProduto;
    }

    // Desconta o desconto concedido na venda
    lucroTotal -= venda.desconto;

    // Desconta a taxa da operadora de cartão
    if (venda.formaPagamento == "Crédito" ||
        venda.formaPagamento == "Débito") {

        double taxa = (venda.total - venda.desconto) *
                      venda.taxa / 100.0;

        lucroTotal -= taxa;
    }

    return lucroTotal;
}
