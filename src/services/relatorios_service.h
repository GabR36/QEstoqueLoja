#ifndef RELATORIOS_SERVICE_H
#define RELATORIOS_SERVICE_H

#include <QObject>
#include <QMap>
#include <QPair>
#include <QStringList>
#include <QDate>
#include "../repository/relatorios_repository.h"

class Relatorios_service : public QObject
{
    Q_OBJECT
public:
    explicit Relatorios_service(QObject *parent = nullptr);

    QMap<QString, int>               buscarQuantVendasPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup);
    QMap<QString, QPair<double,double>> buscarValorVendasPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup);
    QMap<QString, int>               buscarTopProdutosVendidosPeriodo(const QDate &inicio, const QDate &fim);
    QMap<QString, QMap<QString,int>> buscarFormasPagamentoPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup);
    QMap<QString, float>             buscarValoresNfPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup, int tpAmb);
    QMap<QString, float>             produtosMaisLucrativosPeriodo(const QDate &inicio, const QDate &fim);
    bool                             existeProdutoVendido();
    QList<QStringList>               buscarTodosProdutosParaCsv();
    QList<QStringList>               buscarInventario(const QDate &inicio, const QDate &fim, bool somenteNf);

private:
    Relatorios_repository relatoriosRepo;

signals:
};

#endif // RELATORIOS_SERVICE_H
