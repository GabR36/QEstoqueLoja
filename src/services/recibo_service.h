#ifndef RECIBO_SERVICE_H
#define RECIBO_SERVICE_H

#include <QObject>
#include <QSqlDatabase>
#include <QLocale>
#include "produtovenda_service.h"
#include "vendas_service.h"
#include "config_service.h"
#include "entradasvendas_service.h"

class Recibo_service : public QObject
{
    Q_OBJECT
public:
    explicit Recibo_service(QObject *parent = nullptr);
    void imprimirReciboVenda(qlonglong idvenda);
private:
    QSqlDatabase db;
    QLocale portugues;
    ProdutoVenda_service prodVendaServ;
    Vendas_service vendaServ;
    Config_service confServ;
    EntradasVendas_service entradaServ;
signals:
};

#endif // RECIBO_SERVICE_H
