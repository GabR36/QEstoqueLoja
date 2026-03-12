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
    NfceACBR *nfce;
    NfeACBR *nfe;
    WaitDialog* waitDialog = nullptr;
    QString erroNf;
    QString idVenda;
    // QString nomeCli, emailCli, telefoneCli, enderecoCli, cpfCli, numeroCli, bairroCli,
    //     xMunCli, cMunCli, ufCli, cepCli, ieCli;
    // int indIeCLi = 0;
    // bool ehPfCli = false;
    bool emitTodosNf = false;
    QString cStat, xMotivo, msg, nProt;
    QString enviarNfce(NfceACBR *nfce);
    void salvarNfceBD(NfceACBR *nfce);
    QString enviarNfe(NfeACBR *nfe);
    void salvarNfeBD(NfeACBR *nfe);
    QString dhemiRet;
    void enviarEmailNFe(QString nomeCliente, QString emailCliente, QString xmlPath, std::string pdfDanfe);
    ConfigDTO configDTO;
    ProdutoVenda_service prodVendaServ;
    Cliente_service cliServ;
    ClienteDTO CLIENTE;
    Vendas_service vendaServ;
    FiscalEmitter_service fiscalServ;
    NotaFiscal_service notaServ;
signals:
    void gerarEnviarNf();


protected:

};

#endif // PAGAMENTOVENDA_H
