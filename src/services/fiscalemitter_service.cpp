#include "fiscalemitter_service.h"
#include "../util/datautil.h"
#include "../dto/Cliente_dto.h"

FiscalEmitter_service::FiscalEmitter_service(QObject *parent)
    : QObject{parent}
{
    confDTO = confServ.carregarTudo();
}

void FiscalEmitter_service::setRetornoForcado(const QString &retorno)
{
    retornoForcado = retorno;
}

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

FiscalEmitter_service::Resultado FiscalEmitter_service::enviarNfeDevolucaoEntrada(
    qlonglong idNfEntrada,
    QList<ProdutoNotaDTO> produtosNota,
    ClienteDTO cliente)
{
    NotaFiscalDTO notaRef = notaServ.getNotaById(idNfEntrada);
    if(notaRef.chNfe.isEmpty()){
        return {false, FiscalEmitterErro::InfoInsuficiente, "Chave da NF de entrada não encontrada."};
    }

    QList<qlonglong> ids;
    for(const ProdutoNotaDTO &p : produtosNota)
        ids.append(p.id);

    NfeACBR *nfe = new NfeACBR(this, true, true);
    nfe->setNNF(nfe->getProximoNNF());
    nfe->setNfRef(notaRef.chNfe);
    nfe->setProdutosNota(ids);
    nfe->setCliente(cliente);

    NFRetornoDTO notaRet = nfe->gerarEnviarRetorno();

    if(notaRet.cstat.isEmpty()){
        return {false, FiscalEmitterErro::ErroAoEnviar, "Erro ao enviar nota de devolução de entrada."};
    }

    if(notaRet.cstat != "100" && notaRet.cstat != "150"){
        return {false, FiscalEmitterErro::Recusado,
                QString("Nota de Devolução Recusada.\ncStat:%1\nMotivo:%2")
                    .arg(notaRet.cstat, notaRet.xMotivo.isEmpty() ? notaRet.msg : notaRet.xMotivo)};
    }

    NotaFiscalDTO nota;
    nota.atualizadoEm     = DataUtil::getDataAgoraUS();
    nota.chNfe            = notaRet.chNfe;
    nota.cnpjEmit         = notaRet.cnpjEmit;
    nota.cstat            = notaRet.cstat;
    nota.cuf              = notaRet.cuf;
    nota.dhEmi            = notaRet.dhEmi;
    nota.finalidade       = "DEVOLUCAO";
    nota.idEmissorCliente = -1;
    nota.idNfRef          = idNfEntrada;
    nota.idVenda          = -1;
    nota.modelo           = "55";
    nota.nnf              = notaRet.nnf;
    nota.nProt            = notaRet.nProt;
    nota.saida            = true;
    nota.serie            = notaRet.serie;
    nota.tpAmb            = notaRet.tpAmb;
    nota.valorTotal       = notaRet.valorTotal;
    nota.xmlPath          = notaRet.xmlPath;

    if(nota.xmlPath.isEmpty()){
        return {false, FiscalEmitterErro::InfoInsuficiente, "Caminho do arquivo xml está vazio."};
    }
    if(nota.nnf <= 0 || nota.valorTotal <= 0){
        return {false, FiscalEmitterErro::InfoInsuficiente, "NNF ou valor total estão vazios."};
    }

    auto result = notaServ.inserir(nota);
    if(!result.ok){
        return {false, FiscalEmitterErro::Salvar, "Erro ao salvar nota fiscal de devolução."};
    }

    qlonglong idNfDevol = notaServ.getIdFromChave(nota.chNfe);

    for(const ProdutoNotaDTO &p : produtosNota){
        prodNotaServ.marcarComoDevolvido(p.id, idNfDevol);
    }

    return {true, FiscalEmitterErro::Nenhum,
            QString("Nota de Devolução Autorizada e Salva!\ncStat:%1\nMotivo:%2")
                .arg(notaRet.cstat, notaRet.xMotivo.isEmpty() ? notaRet.msg : notaRet.xMotivo),
            idNfDevol};
}

FiscalEmitter_service::Resultado FiscalEmitter_service::enviarNfcePadrao(VendasDTO venda,
                                                                QList<ProdutoVendidoDTO> listaProds,
                                                                         qlonglong nnf,
                                                                         ClienteDTO cliente,
                                                                         bool emitirTodos,
                                                                         bool ignorarNCM
                                                                         ){
    if(cliente.cpf != ""){
        if (cliente.ehPf == true && cliente.cpf.length() != 11){
            return {false, FiscalEmitterErro::QuebraDeRegra, "CPF Inválido. Deve conter 11 dígitos."};
        }
        if (cliente.ehPf == false && cliente.cpf.length() != 14){
            return {false, FiscalEmitterErro::QuebraDeRegra, "CNPJ inválido. Deve conter 14 dígitos."};
        }
    }


    bool algumProdTemNota = false;
    bool algumProdTemNCMErrado = false;
    QStringList produtosNcmInvalido;

    //verifica se os produtos contem nf e ncm invalido
    for(int i = 0; i < listaProds.size(); i++){
        ProdutoDTO produto = prodServ.getProduto(listaProds[i].idProduto);
        if(produto.nf){
            algumProdTemNota = true;
            // se nao esta emitindo todos procurar ncm invalido apenas em produtos nf
            if(!emitirTodos){
                algumProdTemNCMErrado = true;
                produtosNcmInvalido.append(produto.descricao);
            }
        }//se vai emitir todos procura ncm em todos
        if(emitirTodos && (produto.ncm == "00000000" || produto.ncm.isEmpty())){
            algumProdTemNCMErrado = true;
            produtosNcmInvalido.append(produto.descricao);
        }
    }

    if(!algumProdTemNota && !emitirTodos){
        return {true, FiscalEmitterErro::ProdutosSemNF, "Nota Fiscal não enviada pois nenhum "
                                                        "produto selecionado consta como NF."};
    }


    if(algumProdTemNCMErrado && !ignorarNCM){

        QString lista = produtosNcmInvalido.join(", ");

        return {
            false,
            FiscalEmitterErro::NCMInvalido,
            "Os seguintes produtos possuem NCM inválido:\n" + lista
        };
    }

    NfceACBR *nfce = new NfceACBR(this);
    if (!retornoForcado.isEmpty())
        nfce->setRetornoForcado(retornoForcado);
    nfce->setNNF(nnf);
    nfce->setCliente(cliente.cpf, cliente.ehPf);
    nfce->setProdutosVendidosNew(listaProds, emitirTodos);
    nfce->setPagamentoValores(venda.formaPagamento,venda.desconto,venda.valorRecebido,
                              venda.troco, venda.taxa);

    NFRetornoDTO notaRet = nfce->gerarEnviarRetorno();
    qDebug() << "CSTAT: NFCE: " << notaRet.cstat;
    if(notaRet.cstat.isEmpty()){
        return{false, FiscalEmitterErro::ErroAoEnviar, "Erro ao enviar nota."};
    }

    if(notaRet.cstat != "100" || notaRet.cstat != "100"){
        return {false, FiscalEmitterErro::Recusado, QString("Nota Fiscal Recusada.\n"
                                                            "cStat:%1\n"
                                                            "Motivo:%2").arg(notaRet.cstat,
                                                             notaRet.xMotivo.isEmpty() ? notaRet.msg :
                                                                 notaRet.xMotivo)};
    }

    NotaFiscalDTO nota;

    nota.atualizadoEm = DataUtil::getDataAgoraUS();
    nota.chNfe = notaRet.chNfe;
    nota.cnpjEmit = notaRet.cnpjEmit;
    nota.cstat = notaRet.cstat;
    nota.cuf = notaRet.cuf;
    nota.dhEmi = notaRet.dhEmi;
    nota.finalidade = notaRet.finalidade;
    if(cliente.id > 1){
        nota.idEmissorCliente = cliente.id;
    }else{
        nota.idEmissorCliente = -1;
    }
    nota.idNfRef = -1;
    nota.idVenda = venda.id;
    nota.modelo = notaRet.modelo;
    nota.nnf = notaRet.nnf;
    nota.nProt = notaRet.nProt;
    nota.saida = true;
    nota.serie = notaRet.serie;
    nota.tpAmb = notaRet.tpAmb;
    nota.valorTotal = notaRet.valorTotal;
    nota.xmlPath = notaRet.xmlPath;
    qDebug() << "Dhemi nfce:" << nota.dhEmi;


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
        QString msg = "Nota enviada e salva com sucesso.\ncStat:" + nota.cstat;
        return{true, FiscalEmitterErro::Nenhum, msg};
    }

}

FiscalEmitter_service::Resultado FiscalEmitter_service::enviarNFePadrao(VendasDTO venda,
                                                                         QList<ProdutoVendidoDTO> listaProds,
                                                                         qlonglong nnf,
                                                                         ClienteDTO cliente,
                                                                         bool emitirTodos,
                                                                         bool ignorarNCM
                                                                         ){
    if(cliente.cpf != ""){
        if (cliente.ehPf == true && cliente.cpf.length() != 11){
            return {false, FiscalEmitterErro::QuebraDeRegra, "CPF Inválido. Deve conter 11 dígitos."};
        }
        if (cliente.ehPf == false && cliente.cpf.length() != 14){
            return {false, FiscalEmitterErro::QuebraDeRegra, "CNPJ inválido. Deve conter 14 dígitos."};
        }
    }

    if(cliente.nome.isEmpty() || cliente.email.isEmpty() || cliente.endereco.isEmpty() || cliente.cpf.isEmpty() ||
        cliente.numeroEnd <= 0 || cliente.bairro.isEmpty() || cliente.xMun.isEmpty() || cliente.cMun.isEmpty() ||
        cliente.uf.isEmpty() || cliente.cep.isEmpty()){
        return {false, FiscalEmitterErro::QuebraDeRegra, "Informações sobre cliente incompletas."};
    }

    bool algumProdTemNota = false;
    bool algumProdTemNCMErrado = false;
    QStringList produtosNcmInvalido;

    //verifica se os produtos contem nf e ncm invalido
    for(int i = 0; i < listaProds.size(); i++){
        ProdutoDTO produto = prodServ.getProduto(listaProds[i].idProduto);
        if(produto.nf){
            algumProdTemNota = true;
            // se nao esta emitindo todos procurar ncm invalido apenas em produtos nf
            if(!emitirTodos){
                algumProdTemNCMErrado = true;
                produtosNcmInvalido.append(produto.descricao);
            }
        }//se vai emitir todos procura ncm em todos
        if(emitirTodos && (produto.ncm == "00000000" || produto.ncm.isEmpty())){
            algumProdTemNCMErrado = true;
            produtosNcmInvalido.append(produto.descricao);
        }
    }

    if(!algumProdTemNota && !emitirTodos){
        return {true, FiscalEmitterErro::ProdutosSemNF, "Nota Fiscal não enviada pois nenhum "
                                                        "produto selecionado consta como NF."};
    }


    if(algumProdTemNCMErrado && !ignorarNCM){

        QString lista = produtosNcmInvalido.join(", ");

        return {
            false,
            FiscalEmitterErro::NCMInvalido,
            "Os seguintes produtos possuem NCM inválido:\n" + lista
        };
    }

    NfeACBR *nfe = new NfeACBR(this, true, false);
    if (!retornoForcado.isEmpty())
        nfe->setRetornoForcado(retornoForcado);
    nfe->setNNF(nnf);
    nfe->setCliente(cliente);
    nfe->setProdutosVendidosNew(listaProds, emitirTodos);
    nfe->setPagamentoValores(venda.formaPagamento,venda.desconto,venda.valorRecebido,
                              venda.troco, venda.taxa);

    NFRetornoDTO notaRet = nfe->gerarEnviarRetorno();
    qDebug() << "CSTAT: NFE: " << notaRet.cstat;
    if(notaRet.cstat.isEmpty()){
        return{false, FiscalEmitterErro::ErroAoEnviar, "Erro ao enviar nota."};
    }

    if(notaRet.cstat != "100" || notaRet.cstat != "100"){
        return {false, FiscalEmitterErro::Recusado, QString("Nota Fiscal Recusada.\n"
                                                            "cStat:%1\n"
                                                            "Motivo:%2").arg(notaRet.cstat,
                                                             notaRet.xMotivo.isEmpty() ? notaRet.msg :
                                                                 notaRet.xMotivo)};
    }

    NotaFiscalDTO nota;

    nota.atualizadoEm = DataUtil::getDataAgoraUS();
    nota.chNfe = notaRet.chNfe;
    nota.cnpjEmit = notaRet.cnpjEmit;
    nota.cstat = notaRet.cstat;
    nota.cuf = notaRet.cuf;
    nota.dhEmi = notaRet.dhEmi;
    nota.finalidade = notaRet.finalidade;
    if(cliente.id > 1){
        nota.idEmissorCliente = cliente.id;
    }else{
        nota.idEmissorCliente = -1;
    }
    nota.idNfRef = -1;
    nota.idVenda = venda.id;
    nota.modelo = notaRet.modelo;
    nota.nnf = notaRet.nnf;
    nota.nProt = notaRet.nProt;
    nota.saida = true;
    nota.serie = notaRet.serie;
    nota.tpAmb = notaRet.tpAmb;
    nota.valorTotal = notaRet.valorTotal;
    nota.xmlPath = notaRet.xmlPath;
    qDebug() << "Dhemi nfce:" << nota.dhEmi;


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
        QDateTime datavenda = QDateTime::fromString(nota.atualizadoEm, "yyyy-MM-dd HH:mm:ss");
        auto r = emailServ.enviarEmailNFe(cliente.nome, cliente.email, nota.xmlPath,
                                          nfe->getPdfDanfe(), datavenda, confDTO.nomeEmpresa);
        QString msg = "Nota enviada e salva com sucesso. CStat:" + nota.cstat + "\n" + r.msg;
        return{true, FiscalEmitterErro::Nenhum, msg};
    }

}

