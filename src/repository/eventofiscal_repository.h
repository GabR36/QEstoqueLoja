#ifndef EVENTOFISCAL_REPOSITORY_H
#define EVENTOFISCAL_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "../dto/EventoFiscal_dto.h"

class EventoFiscal_repository : public QObject
{
    Q_OBJECT
public:
    explicit EventoFiscal_repository(QObject *parent = nullptr);
    bool inserir(EventoFiscalDTO evento);
    void listarTodos(QSqlQueryModel *model);
private:
    QSqlDatabase db;


signals:
};

#endif // EVENTOFISCAL_REPOSITORY_H
