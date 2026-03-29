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

void ProdutoNota_service::listarPorNota(QSqlQueryModel *model, qlonglong idNf)
{
    prodNotaRepo.listarPorNota(model, idNf);
}

ProdutoNotaDTO ProdutoNota_service::getProdutoNota(qlonglong id){
    return prodNotaRepo.getProdutoNota(id);
}

QString ProdutoNota_service::getXmlPathPorId(qlonglong id){
    return prodNotaRepo.getXmlPathPorId(id);
}

bool ProdutoNota_service::marcarComoAdicionado(qlonglong id){
    return prodNotaRepo.marcarComoAdicionado(id);
}

QVariantMap ProdutoNota_service::getProdutoNotaComXmlPath(qlonglong id){
    return prodNotaRepo.getProdutoNotaComXmlPath(id);
}

bool ProdutoNota_service::marcarComoDevolvido(qlonglong id, qlonglong idNfDevol){
    return prodNotaRepo.marcarComoDevolvido(id, idNfDevol);
}

