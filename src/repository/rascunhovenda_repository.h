#ifndef RASCUNHOVENDA_REPOSITORY_H
#define RASCUNHOVENDA_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include "../dto/RascunhoVenda_dto.h"
#include <QLocale>

class RascunhoVenda_repository : public QObject
{
    Q_OBJECT
public:
    explicit RascunhoVenda_repository(QObject *parent = nullptr);

    bool salvar(const RascunhoVendaDTO &rascunho);
    bool descartar();
    RascunhoVendaDTO carregar();
    bool existe();

private:
    QSqlDatabase db;
    QLocale portugues;

signals:
};

#endif // RASCUNHOVENDA_REPOSITORY_H
