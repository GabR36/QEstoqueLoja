#ifndef RELATORIOS_REPOSITORY_H
#define RELATORIOS_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QMap>
#include <QPair>
#include <QVector>
#include <QStringList>

class Relatorios_repository : public QObject
{
    Q_OBJECT
public:
    explicit Relatorios_repository(QObject *parent = nullptr);

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
    QSqlDatabase db;

signals:
};

#endif // RELATORIOS_REPOSITORY_H
