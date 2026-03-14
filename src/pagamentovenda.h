#ifndef PAGAMENTOVENDA_H
#define PAGAMENTOVENDA_H

#include "pagamento.h"
#include "venda.h"
#include "subclass/waitdialog.h"
#include <qmap.h>
#include "nota/nfceacbr.h"
#include "nota/nfeacbr.h"
#include "services/config_service.h"
#include "services/produtovenda_service.h"
#include "services/cliente_service.h"
#include "services/vendas_service.h"
#include "services/notafiscal_service.h"
#include "services/email_service.h"

class pagamentoVenda : public pagamento
{
    Q_OBJECT
public:
    explicit pagamentoVenda(QList<ProdutoVendidoDTO> listaProdutos, QString total, QString cliente,
                            QString data, qlonglong idCliente, QWidget *parent = nullptr);
    QList<ProdutoVendidoDTO> listaProds;

private slots:
    void on_CBox_ModeloEmit_currentIndexChanged(int index) override;
private:
    void terminarPagamento() override;
    int idCliente;
    WaitDialog* waitDialog = nullptr;
    QString erroNf;
    QString idVenda;
    bool emitTodosNf = false;
    QString cStat, xMotivo, msg, nProt;
    QString dhemiRet;
    ConfigDTO configDTO;
    ProdutoVenda_service prodVendaServ;
    Cliente_service cliServ;
    ClienteDTO CLIENTE;
    Vendas_service vendaServ;
    FiscalEmitter_service fiscalServ;
    NotaFiscal_service notaServ;
    Email_service emailServ;
signals:
    void gerarEnviarNf();


protected:

};

#endif // PAGAMENTOVENDA_H
