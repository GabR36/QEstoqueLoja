#ifndef VENDAS_REPOSITORY_H
#define VENDAS_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>

class Vendas_repository : public QObject
{
    Q_OBJECT
public:
    explicit Vendas_repository(QObject *parent = nullptr);
    qlonglong getQuantidadeComprasCliente(qlonglong idcliente);
    double getValorTotalVendasPrazoCliente(qlonglong idcliente);
private:
    QSqlDatabase db;

signals:
};

#endif // VENDAS_REPOSITORY_H
