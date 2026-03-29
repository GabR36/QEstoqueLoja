#ifndef RELATORIOS_REPOSITORY_H
#define RELATORIOS_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QMap>
#include <QPair>
#include <QVector>
#include <QStringList>
#include <QDate>

enum class Agrupamento { Dia, Mes, Ano };

class Relatorios_repository : public QObject
{
    Q_OBJECT
public:
    explicit Relatorios_repository(QObject *parent = nullptr);

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
    QSqlDatabase db;
    static QString strftimeFormato(Agrupamento agrup);

signals:
};

#endif // RELATORIOS_REPOSITORY_H
