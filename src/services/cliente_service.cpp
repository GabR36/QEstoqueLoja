#include "cliente_service.h"

Cliente_service::Cliente_service(QObject *parent)
    : QObject{parent}
{}

qlonglong Cliente_service::contarQuantosRegistrosPorCPFCNPJ(const QString &cpfcnpj){
    return cliRepo.contarQuantosRegistrosPorCPFCNPJ(cpfcnpj);
}

Cliente_service::Resultado Cliente_service::inserirClienteEmitente(ClienteDTO emissor){

    if(emissor.cpf.length() == 14){
        emissor.ehPf =false;
    }else{
        emissor.ehPf = true;
    }
    if(!emissor.ie.isEmpty()){
        emissor.indIeDest = 1;
    }else{
        emissor.indIeDest = 0;
    }

    if(!cliRepo.inserir(emissor)){
        return {false, ClienteErro::InsercaoInvalida, "Não conseguiu inserir cliente emissor."};
    }else{
        return {true, ClienteErro::Nenhum, ""};
    }
}

qlonglong Cliente_service::getIdFromCpfCnpj(const QString &cpfcnpj){
    return cliRepo.getIdFromCPFCNPJ(cpfcnpj);
}


QSqlQueryModel* Cliente_service::listarClientes(){
    return cliRepo.listarClientes();
}

Cliente_service::Resultado Cliente_service::deletarCliente(qlonglong id){
    if(id > 1){
        if(!cliRepo.deletarCliente(id)){
            return {false, ClienteErro::DeleteFalhou, "Ocorreu um erro ao deletar cliente"};
        }else{
            return {true, ClienteErro::Nenhum, ""};
        }
    }else{
        return {false, ClienteErro::QuebraDeRegra, "Não é possivel deletar o cliente Consumidor."};
    }

}

QSqlQueryModel* Cliente_service::pesquisar(const QString &nome){
    return cliRepo.pesquisar(nome);
}
