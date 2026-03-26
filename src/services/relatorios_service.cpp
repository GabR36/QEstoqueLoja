#include "relatorios_service.h"

Relatorios_service::Relatorios_service(QObject *parent)
    : QObject{parent}
{}

QMap<QString, int> Relatorios_service::buscarQuantVendasPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup)
{
    return relatoriosRepo.buscarQuantVendasPeriodo(inicio, fim, agrup);
}

QMap<QString, QPair<double,double>> Relatorios_service::buscarValorVendasPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup)
{
    return relatoriosRepo.buscarValorVendasPeriodo(inicio, fim, agrup);
}

QMap<QString, int> Relatorios_service::buscarTopProdutosVendidosPeriodo(const QDate &inicio, const QDate &fim)
{
    return relatoriosRepo.buscarTopProdutosVendidosPeriodo(inicio, fim);
}

QMap<QString, QMap<QString,int>> Relatorios_service::buscarFormasPagamentoPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup)
{
    return relatoriosRepo.buscarFormasPagamentoPeriodo(inicio, fim, agrup);
}

QMap<QString, float> Relatorios_service::buscarValoresNfPeriodo(const QDate &inicio, const QDate &fim, Agrupamento agrup, int tpAmb)
{
    return relatoriosRepo.buscarValoresNfPeriodo(inicio, fim, agrup, tpAmb);
}

QMap<QString, float> Relatorios_service::produtosMaisLucrativosPeriodo(const QDate &inicio, const QDate &fim)
{
    return relatoriosRepo.produtosMaisLucrativosPeriodo(inicio, fim);
}

bool Relatorios_service::existeProdutoVendido()
{
    return relatoriosRepo.existeProdutoVendido();
}

QList<QStringList> Relatorios_service::buscarTodosProdutosParaCsv()
{
    return relatoriosRepo.buscarTodosProdutosParaCsv();
}
