#include "produtovenda_service.h"

ProdutoVenda_service::ProdutoVenda_service(QObject *parent)
    : QObject{parent}
{}

QSqlQueryModel* ProdutoVenda_service::listarProdutosVenda(){
    return prodVendaRepo.listarProdutosVenda();
}

QList<ProdutoVendidoDTO> ProdutoVenda_service::getProdutosVendidos(qlonglong idVenda){
    return prodVendaRepo.getProdutosVendidos(idVenda);
}

QSqlQueryModel *ProdutoVenda_service::listarProdutosVendidosFromVenda(qlonglong idvenda){
    return prodVendaRepo.listarProdutosVendidosFromVenda(idvenda);
}
