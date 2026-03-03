#include "produtonota_service.h"
#include <QFile>
#include <QDomDocument>



ProdutoNota_service::ProdutoNota_service(QObject *parent)
    : QObject{parent}
{}

ProdutoNota_service::Resultado ProdutoNota_service::inserirListaProdutos(QList<ProdutoNotaDTO> lista){
    if (lista.isEmpty()) {
        qDebug() << "Nenhum produto encontrado no XML";
        return {false, ProdutoNotaErro::CampoVazio ,"Nenhum produto encontrado no XML"};
    }

    for (const ProdutoNotaDTO &p : lista) {
        if(!prodNotaRepo.inserir(p)){
            return {false, ProdutoNotaErro::InsercaoInvalida,
                    "Erro ao inserir o produto: " + p.descricao};
        }
    }

    return {true, ProdutoNotaErro::Nenhum, ""};
}

