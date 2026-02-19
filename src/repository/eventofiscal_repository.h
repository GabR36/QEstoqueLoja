#ifndef EVENTOFISCAL_REPOSITORY_H
#define EVENTOFISCAL_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include "../dto/EventoFiscal_dto.h"

class EventoFiscal_repository : public QObject
{
    Q_OBJECT
public:
    explicit EventoFiscal_repository(QObject *parent = nullptr);
    bool inserir(EventoFiscalDTO evento);
private:
    QSqlDatabase db;


signals:
};

#endif // EVENTOFISCAL_REPOSITORY_H
