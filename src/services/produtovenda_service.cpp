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

ProdutoVenda_service::Resultado ProdutoVenda_service::deletarProdutoVendido(qlonglong id){
    if(!prodVendaRepo.deletarProdutoVendido(id)){
        return {false, ProdutoVendaErro::Deletar, "Erro ao deletar produto."};
    }else{
        return {true, ProdutoVendaErro::Nenhum, "Produto deletado com sucesso."};
    }
}

ProdutoVenda_service::Resultado ProdutoVenda_service::deletarProdutosVendidosPorIdVenda(qlonglong idvenda){
    if(!prodVendaRepo.deletarPorIdVenda(idvenda)){
        return {false, ProdutoVendaErro::Deletar, "Erro ao deletar produtos vendidos"};
    }else{
        return {true, ProdutoVendaErro::Nenhum, "Produtos Vendidos Deletados com sucesso"};
    }
}

bool ProdutoVenda_service::temApenasUmProduto(qlonglong idvenda){
    int quantidadeProds = prodVendaRepo.contarProdutosVendidosFromVenda(idvenda);
    qDebug() << "Quantidade de produto da venda: " << quantidadeProds;
    if(quantidadeProds == 1){

        return true;
    }else{
        return false;
    }
}

ProdutoVendidoDTO ProdutoVenda_service::getProdutoVendido(qlonglong id){
    return prodVendaRepo.getProdutoVendido(id);
}



