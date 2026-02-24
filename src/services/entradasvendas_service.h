#ifndef ENTRADASVENDAS_SERVICE_H
#define ENTRADASVENDAS_SERVICE_H

#include <QObject>
#include "../repository/entradasvendas_repository.h"
#include <QLocale>
#include "../dto/EntradaVenda_dto.h"
#include <QList>

enum class EntradasVendasErro {
    Nenhum,
    CampoVazio,
    Banco,
    InsercaoInvalida,
    DeleteFalhou,
    QuebraDeRegra
};

class EntradasVendas_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        EntradasVendasErro erro = EntradasVendasErro::Nenhum;
        QString msg;
    };
    explicit EntradasVendas_service(QObject *parent = nullptr);
    double getValorUltimoPagamentoFromCliente(qlonglong idcliente);
    double getValorTotalEntradasFromClientes(qlonglong idcliente);
    QDateTime getDataUltimoPagamentoFromCliente(qlonglong idcliente);
    QList<EntradaVendaDTO> getEntradasFromVenda(qlonglong idvenda);
private:
    EntradasVendas_repository entradaRepo;
    QLocale locale;

signals:
};

#endif // ENTRADASVENDAS_SERVICE_H
