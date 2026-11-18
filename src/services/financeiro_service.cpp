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

double Financeiro_service::getLucroEstimadoVendaDto(
    VendasDTO venda,
    QList<ProdutoVendidoDTO> produtos)
{
    double lucroTotal = 0.0;

    QList<ProdutoDTO> prods =
        prodServ.getListaProdutosViaListaVendidos(produtos);

    qDebug() << "debug produtos getlucroestimadovendadto";
    for (const ProdutoDTO &produto : prods) {
        qDebug() << "ID produto:" << produto.id;
        qDebug() << "Quantidade:" << produto.quantidade;
        qDebug() << "preco:" << produto.preco;
        qDebug() << "descricao:" << produto.descricao;
        qDebug() << "porcent lucro:" << produto.percentLucro;
        qDebug() << "preco fornecedor:" << produto.precoFornecedor;
        qDebug() << "----------------------";
    }

    // Calcula o lucro individual de cada produto
    for (const ProdutoVendidoDTO &produto : produtos) {

        for (const ProdutoDTO &prod1 : prods) {
            if (prod1.id == produto.idProduto) {

                double lucroProduto =
                    produto.quantidade *
                    (produto.precoVendido - prod1.precoFornecedor);

                lucroTotal += lucroProduto;
                break;
            }
        }
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
