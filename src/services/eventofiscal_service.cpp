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


