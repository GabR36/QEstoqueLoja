#ifndef EVENTOFISCAL_SERVICE_H
#define EVENTOFISCAL_SERVICE_H

#include <QObject>
#include <QSqlQueryModel>
#include "../repository/eventofiscal_repository.h"
#include "../dto/EventoFiscal_dto.h"
#include "../nota/cancelnf.h"

enum class EventoFiscalErro{
    Nenhum,
    Banco,
    InsercaoInvalida,
    UpdateInvalido,
    EnvioEvento,
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
    EventoFiscal_service::Resultado enviarCancelamento(qlonglong idnf);
    void listarTodos(QSqlQueryModel *model);
private:
    EventoFiscal_repository eventoRepo;

signals:
};

#endif // EVENTOFISCAL_SERVICE_H
