#include "relatorios_service.h"

Relatorios_service::Relatorios_service(QObject *parent)
    : QObject{parent}
{}

QStringList Relatorios_service::buscarAnosDisponiveis()
{
    return relatoriosRepo.buscarAnosDisponiveis();
}

QMap<QString, int> Relatorios_service::buscarVendasPorMes()
{
    return relatoriosRepo.buscarVendasPorMes();
}

QMap<QString, int> Relatorios_service::buscarVendasPorMesAno(const QString &ano)
{
    return relatoriosRepo.buscarVendasPorMesAno(ano);
}

QMap<QString, int> Relatorios_service::buscarVendasPorDiaMesAno(const QString &ano, const QString &mes)
{
    return relatoriosRepo.buscarVendasPorDiaMesAno(ano, mes);
}

QMap<QString, double> Relatorios_service::buscarValorVendasPorDiaMesAno(const QString &ano, const QString &mes)
{
    return relatoriosRepo.buscarValorVendasPorDiaMesAno(ano, mes);
}

QMap<QString, QPair<double, double>> Relatorios_service::buscarValorVendasPorMesAno(const QString &ano)
{
    return relatoriosRepo.buscarValorVendasPorMesAno(ano);
}

QMap<QString, int> Relatorios_service::buscarTopProdutosVendidos()
{
    return relatoriosRepo.buscarTopProdutosVendidos();
}

QMap<QString, QVector<int>> Relatorios_service::buscarFormasPagamentoPorAno(const QString &ano)
{
    return relatoriosRepo.buscarFormasPagamentoPorAno(ano);
}

QMap<QString, float> Relatorios_service::buscarValoresNfAno(const QString &ano, int tpAmb)
{
    return relatoriosRepo.buscarValoresNfAno(ano, tpAmb);
}

QMap<QString, float> Relatorios_service::produtosMaisLucrativosAno(const QString &ano)
{
    return relatoriosRepo.produtosMaisLucrativosAno(ano);
}

bool Relatorios_service::existeProdutoVendido()
{
    return relatoriosRepo.existeProdutoVendido();
}

QList<QStringList> Relatorios_service::buscarTodosProdutosParaCsv()
{
    return relatoriosRepo.buscarTodosProdutosParaCsv();
}
