#include "eventofiscal_service.h"


EventoFiscal_service::EventoFiscal_service(QObject *parent)
    : QObject{parent}
{}

EventoFiscal_service::Resultado EventoFiscal_service::inserir(EventoFiscalDTO evento){
    if(eventoRepo.inserir(evento)){
        qDebug() << "Evento Fiscal Inserido";
        return {true, EventoFiscalErro::Nenhum, ""};
    }else{
        return {false, EventoFiscalErro::InsercaoInvalida, "Falha ao inserir o evento fiscal"};
    }
}


EventoFiscal_service::Resultado EventoFiscal_service::enviarCancelamento(qlonglong idnf){
    cancelNf *cancel = new cancelNf(this, idnf);
    EventoFiscalDTO evento = cancel->GerarEnviarRetorno();

    if(evento.cstat != "135"){
        QString just = evento.justificativa;
        if(just.isEmpty()){
            just = "Erro desconhecido";
        }
        QString msgErro = "Erro ao enviar cancelamento:\n" + just;
        return {false, EventoFiscalErro::EnvioEvento, msgErro};
    }else{
        if(!eventoRepo.inserir(evento)){
            return {false, EventoFiscalErro::Banco, "Problema com banco de dados."};
        }else{
            return {true, EventoFiscalErro::Nenhum, "Evento Cancelamento enviado com sucesso."};
        }
    }
}
