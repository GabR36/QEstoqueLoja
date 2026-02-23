#include "cliente_service.h"
#include <QRegularExpression>

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


Cliente_service::Resultado Cliente_service::inserirCliente(ClienteDTO cliente)
{
    Resultado r;

    // 1️ Nome obrigatório
    if (cliente.nome.trimmed().isEmpty()) {
        r.ok = false;
        r.erro = ClienteErro::CampoVazio;
        r.msg = "Nome é obrigatório.";
        return r;
    }

    // 2️ CPF/CNPJ (opcional)
    if (!cliente.cpf.trimmed().isEmpty()) {

        for (const QChar &c : cliente.cpf) {
            if (!c.isDigit()) {
                r.ok = false;
                r.erro = ClienteErro::QuebraDeRegra;
                r.msg = "CPF/CNPJ deve conter apenas números.";
                return r;
            }
        }

        if (cliente.ehPf) {
            if (cliente.cpf.length() != 11) {
                r.ok = false;
                r.erro = ClienteErro::QuebraDeRegra;
                r.msg = "CPF deve conter 11 dígitos.";
                return r;
            }
        } else {
            if (cliente.cpf.length() != 14) {
                r.ok = false;
                r.erro = ClienteErro::QuebraDeRegra;
                r.msg = "CNPJ deve conter 14 dígitos.";
                return r;
            }
        }

        // Duplicidade só verifica se CPF foi informado
        if (cliRepo.contarQuantosRegistrosPorCPFCNPJ(cliente.cpf) > 0) {
            r.ok = false;
            r.erro = ClienteErro::InsercaoInvalida;
            r.msg = "Já existe cliente com este CPF/CNPJ.";
            return r;
        }
    }

    // 3️ Email opcional
    if (!cliente.email.trimmed().isEmpty()) {
        QRegularExpression emailRegex(R"((\w+)(\.\w+)*@(\w+)((\.\w+)+))");
        if (!emailRegex.match(cliente.email).hasMatch()) {
            r.ok = false;
            r.erro = ClienteErro::QuebraDeRegra;
            r.msg = "Email inválido.";
            return r;
        }
    }

    // 4️ Telefone opcional
    if (!cliente.telefone.trimmed().isEmpty()) {
        if (cliente.telefone.length() < 10) {
            r.ok = false;
            r.erro = ClienteErro::QuebraDeRegra;
            r.msg = "Telefone inválido.";
            return r;
        }

        for (const QChar &c : cliente.telefone) {
            if (!c.isDigit()) {
                r.ok = false;
                r.erro = ClienteErro::QuebraDeRegra;
                r.msg = "Telefone deve conter apenas números.";
                return r;
            }
        }
    }

    // if(!cliente.ie.isEmpty()){
    //     if (cliente.indIeDest == 0) {
    //         r.ok = false;
    //         r.erro = ClienteErro::QuebraDeRegra;
    //         r.msg = "Não contribuinte com Inscrição Estadual.";
    //         return
    // }

    if (!cliRepo.inserir(cliente)) {
        r.ok = false;
        r.erro = ClienteErro::Banco;
        r.msg = "Erro ao inserir cliente no banco.";
        return r;
    }

    r.ok = true;
    r.msg = "Cliente inserido com sucesso.";
    return r;
}


ClienteDTO Cliente_service::getClienteByID(qlonglong id){
    return cliRepo.getClienteByID(id);
}

Cliente_service::Resultado Cliente_service::updateCliente(qlonglong id, ClienteDTO cliente){
    Resultado r;

    // 1️ Nome obrigatório
    if (cliente.nome.trimmed().isEmpty()) {
        r.ok = false;
        r.erro = ClienteErro::CampoVazio;
        r.msg = "Nome é obrigatório.";
        return r;
    }

    // 2️ CPF/CNPJ (opcional)
    if (!cliente.cpf.trimmed().isEmpty()) {

        for (const QChar &c : cliente.cpf) {
            if (!c.isDigit()) {
                r.ok = false;
                r.erro = ClienteErro::QuebraDeRegra;
                r.msg = "CPF/CNPJ deve conter apenas números.";
                return r;
            }
        }

        if (cliente.ehPf) {
            if (cliente.cpf.length() != 11) {
                r.ok = false;
                r.erro = ClienteErro::QuebraDeRegra;
                r.msg = "CPF deve conter 11 dígitos.";
                return r;
            }
        } else {
            if (cliente.cpf.length() != 14) {
                r.ok = false;
                r.erro = ClienteErro::QuebraDeRegra;
                r.msg = "CNPJ deve conter 14 dígitos.";
                return r;
            }
        }

    }

    // 3️ Email opcional
    if (!cliente.email.trimmed().isEmpty()) {
        QRegularExpression emailRegex(R"((\w+)(\.\w+)*@(\w+)((\.\w+)+))");
        if (!emailRegex.match(cliente.email).hasMatch()) {
            r.ok = false;
            r.erro = ClienteErro::QuebraDeRegra;
            r.msg = "Email inválido.";
            return r;
        }
    }

    // 4️ Telefone opcional
    if (!cliente.telefone.trimmed().isEmpty()) {
        if (cliente.telefone.length() < 10) {
            r.ok = false;
            r.erro = ClienteErro::QuebraDeRegra;
            r.msg = "Telefone inválido.";
            return r;
        }

        for (const QChar &c : cliente.telefone) {
            if (!c.isDigit()) {
                r.ok = false;
                r.erro = ClienteErro::QuebraDeRegra;
                r.msg = "Telefone deve conter apenas números.";
                return r;
            }
        }
    }

    // if(!cliente.ie.isEmpty()){
    //     if (cliente.indIeDest == 0) {
    //         r.ok = false;
    //         r.erro = ClienteErro::QuebraDeRegra;
    //         r.msg = "Não contribuinte com Inscrição Estadual.";
    //         return
    // }

    if (!cliRepo.updateCliente(id, cliente)) {
        r.ok = false;
        r.erro = ClienteErro::Banco;
        r.msg = "Erro ao atualizar cliente no banco.";
        return r;
    }else{
        return {true, ClienteErro::Nenhum, ""};
    }
}
