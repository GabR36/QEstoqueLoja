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

qlonglong NotaFiscal_service::getProximoNNF(QString serie, bool tpAmb, qlonglong nnfConfigurado){
    return notaRepo.getProximoNNF(serie, tpAmb, nnfConfigurado);
}

NotaFiscalDTO NotaFiscal_service::getNotaNormalFromIdVenda(qlonglong idvenda){
    return notaRepo.getNotaNormalFromIdVenda(idvenda);
}

NotaFiscal_service::Resultado NotaFiscal_service::inserir(NotaFiscalDTO nota){
    if(!notaRepo.inserir(nota)){
        return {false, NotaErro::Salvar, "Erro ao inserir Nota Fiscal"};
    }else{
        return {true, NotaErro::Nenhum, "Nota salva com sucesso."};
    }
}

bool NotaFiscal_service::temNotaNormal(qlonglong idvenda){
    qlonglong id = notaRepo.getIdNotaNormalFromIdVenda(idvenda);

    if(id != -1){
        return true;
    }else{
        return false;
    }
}
