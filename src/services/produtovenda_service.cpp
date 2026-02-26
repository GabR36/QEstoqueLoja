#include "produtovenda_service.h"

ProdutoVenda_service::ProdutoVenda_service(QObject *parent)
    : QObject{parent}
{}

void ProdutoVenda_service::listarProdutosVenda(QSqlQueryModel *model){
    return prodVendaRepo.listarProdutosVenda(model);
}

QList<ProdutoVendidoDTO> ProdutoVenda_service::getProdutosVendidos(qlonglong idVenda){
    return prodVendaRepo.getProdutosVendidos(idVenda);
}

void ProdutoVenda_service::listarProdutosVendidosFromVenda(qlonglong idvenda, QSqlQueryModel *model){
    return prodVendaRepo.listarProdutosVendidosFromVenda(idvenda, model);
}
