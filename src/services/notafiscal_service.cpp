#include "notafiscal_service.h"
#include <qdebug.h>

NotaFiscal_service::NotaFiscal_service(QObject *parent)
    : QObject{parent}
{}

NotaFiscal_service::Resultado NotaFiscal_service::salvarResNfe(NotaFiscalDTO resumoNota){
    if(notaRepo.salvarResNFe(resumoNota)){
        qDebug() << "Resumo nota salvo com sucesso!";
        return {true, NotaErro::Nenhum, ""};
    }else{
        return {false, NotaErro::Salvar, "Erro ao salvar resumo NFe."};
    }

}

qlonglong NotaFiscal_service::getIdFromChave(QString chnfe){
    return notaRepo.getIdFromChave(chnfe);
}

NotaFiscal_service::Resultado NotaFiscal_service::updateWhereChave(NotaFiscalDTO dto,
                                                                   QString chave){
    if(notaRepo.updateWhereChave(dto,chave)){
        return {true, NotaErro::Nenhum, ""};
    }else{
        return {false, NotaErro::Update, "Erro ao atualizar Nota Fiscal"};
    }
}

qlonglong NotaFiscal_service::getIdFromIdVenda(qlonglong idvenda){
    return notaRepo.getIdFromIdVenda(idvenda);
}

