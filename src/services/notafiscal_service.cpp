#include "notafiscal_service.h"
#include <qdebug.h>

NotaFiscal_service::NotaFiscal_service(QObject *parent)
    : QObject{parent}
{
    confDTO = confServ.carregarTudo();
}

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

qlonglong NotaFiscal_service::getProximoNNF(bool tpAmb, ModeloNota mod){
    QString serie = "1"; // ou algum valor padrão
    return getProximoNNF(serie, tpAmb, mod);
}

qlonglong NotaFiscal_service::getProximoNNF(QString serie, bool tpAmb, ModeloNota mod){
    qlonglong nnf;

    if(mod == ModeloNota::NFCe){
        if(tpAmb){
            nnf = confDTO.nnfProdFiscal;
        }else{
            nnf = confDTO.nnfHomologFiscal;
        }

        return notaRepo.getProximoNNF65(serie, tpAmb, nnf);

    } else if(mod == ModeloNota::NFe){
        if(tpAmb){
            nnf = confDTO.nnfProdNfeFiscal;
        }else{
            nnf = confDTO.nnfHomologNfeFiscal;
        }

        return notaRepo.getProximoNNF55(serie, tpAmb, nnf);
    }

    return 0;
}

NotaFiscalDTO NotaFiscal_service::getNotaNormalFromIdVenda(qlonglong idvenda){
    return notaRepo.getNotaNormalFromIdVenda(idvenda);
}

NotaFiscalDTO NotaFiscal_service::getNotaById(qlonglong id){
    return notaRepo.getNotaById(id);
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

void NotaFiscal_service::listarEntradas(QSqlQueryModel *model, const QString &de, const QString &ate)
{
    notaRepo.listarEntradas(model, de, ate);
}

void NotaFiscal_service::listarMonitor(QSqlQueryModel *model, const QStringList &finalidades)
{
    notaRepo.listarMonitor(model, finalidades);
}
