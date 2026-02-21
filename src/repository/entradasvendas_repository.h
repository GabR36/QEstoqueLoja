#ifndef ENTRADASVENDAS_REPOSITORY_H
#define ENTRADASVENDAS_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>

class EntradasVendas_repository : public QObject
{
    Q_OBJECT
public:
    explicit EntradasVendas_repository(QObject *parent = nullptr);
    QDateTime getDataUltimoPagamentoFromCliente(qlonglong idcliente);
    double getValorUltimoPagamentoFromCliente(qlonglong idcliente);
    double getValorTotalEntradasFromClientes(qlonglong idcliente);
private:
    QSqlDatabase db;

signals:
};

#endif // ENTRADASVENDAS_REPOSITORY_H
