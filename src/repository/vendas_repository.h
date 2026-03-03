#ifndef VENDAS_REPOSITORY_H
#define VENDAS_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QPair>
#include <QDate>
#include "../dto/Vendas_dto.h"
#include "../dto/ResumoVendas_dto.h"
#include "../util/vendasutil.h"

class Vendas_repository : public QObject
{
    Q_OBJECT
public:
    explicit Vendas_repository(QObject *parent = nullptr);
    qlonglong getQuantidadeComprasCliente(qlonglong idcliente);
    double getValorTotalVendasPrazoCliente(qlonglong idcliente);
    void listarVendas(QSqlQueryModel *model);
    QPair<QDate, QDate> getMinMaxData();
    VendasDTO getVenda(qlonglong id);
    ResumoVendasDTO calcularResumo(const QString &dataDe, const QString &dataAte, bool somentePrazo, qlonglong idCliente);
    void listarVendasDeAteFormaPagamento(QSqlQueryModel *model, const QString &de, const QString &ate, VendasUtil::VendasFormaPagamento formaPag);
    bool deletarVenda(qlonglong id);
    bool updateNewTotalTrocoValorFinal(double total, double troco, double valorFinal, qlonglong id);
private:
    QSqlDatabase db;

signals:
};

#endif // VENDAS_REPOSITORY_H
