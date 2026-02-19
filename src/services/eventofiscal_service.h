#ifndef EVENTOFISCAL_SERVICE_H
#define EVENTOFISCAL_SERVICE_H

#include <QObject>
#include "../repository/eventofiscal_repository.h"
#include "../dto/EventoFiscal_dto.h"

enum class EventoFiscalErro{
    Nenhum,
    Banco,
    InsercaoInvalida,
    UpdateInvalido
};


class EventoFiscal_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        EventoFiscalErro erro = EventoFiscalErro::Nenhum;
        QString msg;
    };
    explicit EventoFiscal_service(QObject *parent = nullptr);
    EventoFiscal_service::Resultado inserir(EventoFiscalDTO evento);
private:
    EventoFiscal_repository eventoRepo;

signals:
};

#endif // EVENTOFISCAL_SERVICE_H
