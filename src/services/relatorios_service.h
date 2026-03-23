#ifndef RELATORIOS_SERVICE_H
#define RELATORIOS_SERVICE_H

#include <QObject>
#include <QMap>
#include <QPair>
#include <QVector>
#include <QStringList>
#include "../repository/relatorios_repository.h"

class Relatorios_service : public QObject
{
    Q_OBJECT
public:
    explicit Relatorios_service(QObject *parent = nullptr);

    QStringList buscarAnosDisponiveis();
    QMap<QString, int>              buscarVendasPorMes();
    QMap<QString, int>              buscarVendasPorMesAno(const QString &ano);
    QMap<QString, int>              buscarVendasPorDiaMesAno(const QString &ano, const QString &mes);
    QMap<QString, double>           buscarValorVendasPorDiaMesAno(const QString &ano, const QString &mes);
    QMap<QString, QPair<double,double>> buscarValorVendasPorMesAno(const QString &ano);
    QMap<QString, int>              buscarTopProdutosVendidos();
    QMap<QString, QVector<int>>     buscarFormasPagamentoPorAno(const QString &ano);
    QMap<QString, float>            buscarValoresNfAno(const QString &ano, int tpAmb);
    QMap<QString, float>            produtosMaisLucrativosAno(const QString &ano);
    bool                            existeProdutoVendido();
    QList<QStringList>              buscarTodosProdutosParaCsv();

private:
    Relatorios_repository relatoriosRepo;

signals:
};

#endif // RELATORIOS_SERVICE_H
