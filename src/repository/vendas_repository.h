#ifndef VENDAS_REPOSITORY_H
#define VENDAS_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QPair>
#include <QDate>
#include "../dto/Vendas_dto.h"


class Vendas_repository : public QObject
{
    Q_OBJECT
public:
    explicit Vendas_repository(QObject *parent = nullptr);
    qlonglong getQuantidadeComprasCliente(qlonglong idcliente);
    double getValorTotalVendasPrazoCliente(qlonglong idcliente);
    QSqlQueryModel *listarVendas();
    QPair<QDate, QDate> getMinMaxData();
    VendasDTO getVenda(qlonglong id);
private:
    QSqlDatabase db;

signals:
};

#endif // VENDAS_REPOSITORY_H
