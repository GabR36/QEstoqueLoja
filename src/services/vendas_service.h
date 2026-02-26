#ifndef VENDAS_SERVICE_H
#define VENDAS_SERVICE_H

#include <QObject>
#include "../repository/vendas_repository.h"
#include <QSqlQueryModel>
#include <QPair>
#include <QDate>
#include "../dto/Vendas_dto.h"
#include "../dto/ResumoVendas_dto.h"

enum class VendasErro {
    Nenhum,
    CampoVazio,
    Banco,
    InsercaoInvalida,
    DeleteFalhou,
    QuebraDeRegra
};

class Vendas_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        VendasErro erro = VendasErro::Nenhum;
        QString msg;
    };
    explicit Vendas_service(QObject *parent = nullptr);
    qlonglong getQuantidadeComprasCliente(qlonglong idcli);
    double getValorTotalVendasPrazoCliente(qlonglong idcliente);
    void listarVendas(QSqlQueryModel *model);
    QPair<QDate, QDate> getMinMaxData();
    VendasDTO getVenda(qlonglong id);
    ResumoVendasDTO calcularResumo(const QString &dataDe, const QString &dataAte, bool somentePrazo, qlonglong idCliente);
    void listarVendasDeAteFormaPag(QSqlQueryModel *model, QString de, QString ate, VendasUtil::VendasFormaPagamento formaPag);
private:
    Vendas_repository vendasRepo;

signals:
};

#endif // VENDAS_SERVICE_H
