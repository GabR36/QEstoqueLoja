#ifndef VENDAS_SERVICE_H
#define VENDAS_SERVICE_H

#include <QObject>
#include "../repository/vendas_repository.h"
#include <QSqlQueryModel>
#include <QPair>
#include <QDate>
#include "../dto/Vendas_dto.h"
#include "../dto/ResumoVendas_dto.h"
#include "eventofiscal_service.h"
#include "notafiscal_service.h"
#include "produtovenda_service.h"
#include "Produto_service.h"
#include "entradasvendas_service.h"


enum class VendasErro {
    Nenhum,
    CampoVazio,
    Banco,
    InsercaoInvalida,
    DeleteFalhou,
    QuebraDeRegra,
    EventoFiscal,
    Produto,
    NotaFiscal,
    ProdutoVendido,
    EntradasVendas,
    UpdateFalhou

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
    Vendas_service::Resultado deletarVenda(qlonglong id);
    Vendas_service::Resultado deletarVendaRegraNegocio(qlonglong idVenda, bool cancelarNf);
    bool vendaPossuiNota(qlonglong idVenda);
    Vendas_service::Resultado updateNewTotalTrocoValorFinal(double total, double troco, double valorFinal, qlonglong id);
    Vendas_service::Resultado devolverProdutoRegraNegocio(qlonglong idProdVend, qlonglong idVenda);
private:
    QSqlDatabase db;
    Vendas_repository vendasRepo;
    EventoFiscal_service eventoServ;
    ProdutoVenda_service prodVendaServ;
    NotaFiscal_service notaServ;
    Produto_Service prodServ;
    EntradasVendas_service entradaServ;

signals:
};

#endif // VENDAS_SERVICE_H
