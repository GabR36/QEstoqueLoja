#include "eventofiscal_service.h"
#include "../nota/eventocienciaop.h"
#include "notafiscal_service.h"
#include "../nota/eventocartacorrecao.h"


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

EventoFiscal_service::Resultado EventoFiscal_service::enviarCienciaOp(QString chnfe, QString &retornoforcado){
    EventoCienciaOP *evento = new EventoCienciaOP(this, chnfe);
    if(!retornoforcado.isEmpty()){
        evento->setRetornoForcado(retornoforcado);
    }
    EventoFiscalDTO info =  evento->gerarEnviarRetorno();
    if(info.cstat == "128" || info.cstat == "135" || info.cstat == "136"){

        qlonglong idnf;
        idnf = nfServ.getIdFromChave(chnfe);
        EventoFiscalDTO evento;
        evento = info;
        evento.idNf = idnf;

        auto result = inserir(evento);
        if(!result.ok){
            return {false, EventoFiscalErro::InsercaoInvalida, result.msg};
        }

        return {true, EventoFiscalErro::Nenhum, "Evento Enviado e Salvo com Sucesso."};
    }else{
        return {false, EventoFiscalErro::EventoRecusadoSefaz, "Evento Recusado Sefaz. CStat: " + info.cstat};
    }
}

void EventoFiscal_service::listarTodos(QSqlQueryModel *model)
{
    eventoRepo.listarTodos(model);
}

QMap<QString, int> EventoFiscal_service::contarPorTipo(QDateTime dtIni, QDateTime dtFim)
{
    return eventoRepo.contarPorTipo(dtIni, dtFim);
}

QList<QPair<QString, QString>> EventoFiscal_service::buscarXmlsPorPeriodo(QDateTime dtIni, QDateTime dtFim)
{
    return eventoRepo.buscarXmlsPorPeriodo(dtIni, dtFim);
}


EventoFiscal_service::Resultado EventoFiscal_service::enviarCCE(QString chnfe, int nseq,
                                                                QString correcao,
                                                                QString retornoforcado){
    qDebug() << "Tentando enviar CCE";

    if(correcao.length() < 15){
        return {false,  EventoFiscalErro::QuebraDeRegra, "A correção deve ter no mínimo "
                                                        "15 caracteres"};
    }

    if(chnfe.isEmpty() || nseq < 1){
        return {false,  EventoFiscalErro::QuebraDeRegra, "Campos 'Chave NF' ou 'NSec' incorretos."};
    }
    EventoCartaCorrecao *cce = new EventoCartaCorrecao(this, chnfe, nseq, correcao);
    if(!retornoforcado.isEmpty()){
        cce->setRetornoForcado(retornoforcado);
    }
    EventoFiscalDTO info =  cce->gerarEnviarRetorno();
    qDebug() << "JUSTIFICATIVA EVENTO CCE:" << info.justificativa;

    if(info.cstat == "128" || info.cstat == "135" || info.cstat == "136"){

        qlonglong idnf;
        idnf = nfServ.getIdFromChave(chnfe);
        EventoFiscalDTO evento;
        evento = info;
        evento.idNf = idnf;

        auto result = inserir(evento);
        if(!result.ok){
            return {false, EventoFiscalErro::InsercaoInvalida, result.msg};
        }

        return {true, EventoFiscalErro::Nenhum, "Evento Enviado e Salvo com Sucesso."};
    }else if(info.cstat != "-1" && !info.cstat.isEmpty()){
        return {false, EventoFiscalErro::EventoRecusadoSefaz, "Evento Recusado Sefaz. CStat: "
                                                                  + info.cstat + "\n" + info.justificativa};
    }else{
        return {false, EventoFiscalErro::Desconhecido, info.justificativa};

    }
}
