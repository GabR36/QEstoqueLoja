#include "fiscalemitter_service.h"
#include "../util/datautil.h"

FiscalEmitter_service::FiscalEmitter_service(QObject *parent)
    : QObject{parent}
{}

FiscalEmitter_service::Resultado FiscalEmitter_service::enviarNfeDevolucaoPadrao(qlonglong idvenda,
                                                                QList<ProdutoVendidoDTO> listaProds){

    NotaFiscalDTO notaRef = notaServ.getNotaNormalFromIdVenda(idvenda);

    //Verifica se tem algum produto com nota para evitar emitir nota sem nada
    bool algumProdTemNota = false;
    for(int i = 0; i < listaProds.size(); i++){
        ProdutoDTO produto = prodServ.getProduto(listaProds[i].idProduto);
        if(produto.nf){
            algumProdTemNota = true;
            break;
        }
    }

    if(algumProdTemNota){
        NfeACBR *nfe = new NfeACBR(this, false, true);
        nfe->setNNF(nfe->getProximoNNF());
        nfe->setNfRef(notaRef.chNfe);
        nfe->setProdutosVendidosNew(listaProds, false);

        NFRetornoDTO notaRet = nfe->gerarEnviarRetorno();
        NotaFiscalDTO nota;
        if(notaRet.cstat.isEmpty()){
            return{false, FiscalEmitterErro::ErroAoEnviar, "Erro ao enviar nota."};
        }

        if(notaRet.cstat != "100" || notaRet.cstat != "100"){
            return {false, FiscalEmitterErro::Recusado, QString("Nota de Devolução Recusada.\n"
                                                            "cStat:%1\n"
                                                            "Motivo:%2").arg(notaRet.cstat,
                                                             notaRet.xMotivo.isEmpty() ? notaRet.msg :
                                                                     notaRet.xMotivo)};
        }
        nota.atualizadoEm = DataUtil::getDataAgoraUS();
        nota.chNfe = notaRet.chNfe;
        nota.cnpjEmit = notaRet.cnpjEmit;
        nota.cstat = notaRet.cstat;
        nota.cuf = notaRet.cuf;
        nota.dhEmi = notaRet.dhEmi;
        nota.finalidade = notaRet.finalidade;
        nota.idEmissorCliente = -1;
        nota.idNfRef = notaServ.getIdFromChave(notaRef.chNfe);
        nota.idVenda = idvenda;
        nota.modelo = notaRet.modelo;
        nota.nnf = notaRet.nnf;
        nota.nProt = notaRet.nProt;
        nota.saida = false;
        nota.serie = notaRet.serie;
        nota.tpAmb = notaRet.tpAmb;
        nota.valorTotal = notaRet.valorTotal;
        nota.xmlPath = notaRet.xmlPath;

        if(nota.xmlPath.isEmpty()){
            return {false, FiscalEmitterErro::InfoInsuficiente, "Caminho do arquivo xml está vazio"};
        }
        if(nota.nnf <= 0 || nota.valorTotal <= 0){
            return {false, FiscalEmitterErro::InfoInsuficiente, "NNF ou valor total estão vazios"};

        }
        auto result = notaServ.inserir(nota);
        if(!result.ok){
            return {false, FiscalEmitterErro::Salvar, "Erro ao salvar nota fiscal."};
        }else{
            return{true, FiscalEmitterErro::Nenhum, "Nota enviada e salva com sucesso."};
        }
    }else{
        return{true, FiscalEmitterErro::ProdutosSemNF, "Nota de devolução não enviada pois nenhum "
                                                        "produto selecionado consta como NF."};
    }

}
