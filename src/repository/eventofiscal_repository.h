#ifndef EVENTOFISCAL_REPOSITORY_H
#define EVENTOFISCAL_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QMap>
#include <QList>
#include <QPair>
#include <QDateTime>
#include "../dto/EventoFiscal_dto.h"

class EventoFiscal_repository : public QObject
{
    Q_OBJECT
public:
    explicit EventoFiscal_repository(QObject *parent = nullptr);
    bool inserir(EventoFiscalDTO evento);
    void listarTodos(QSqlQueryModel *model);
    QMap<QString, int>             contarPorTipo(QDateTime dtIni, QDateTime dtFim);
    QList<QPair<QString, QString>> buscarXmlsPorPeriodo(QDateTime dtIni, QDateTime dtFim);
private:
    QSqlDatabase db;


signals:
};

#endif // EVENTOFISCAL_REPOSITORY_H
