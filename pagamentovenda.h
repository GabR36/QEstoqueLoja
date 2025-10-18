#ifndef PAGAMENTOVENDA_H
#define PAGAMENTOVENDA_H

#include "pagamento.h"
#include "venda.h"
#include "subclass/waitdialog.h"
#include <qmap.h>
#include "nota/nfceacbr.h"
#include "nota/nfeacbr.h"

class pagamentoVenda : public pagamento
{
    Q_OBJECT
public:
    explicit pagamentoVenda(QList<QList<QVariant>> listaProdutos, QString total, QString cliente, QString data, int idCliente, QWidget *parent = nullptr);
    QList<QList<QVariant>> rowDataList;

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
    QString nomeCli, emailCli, telefoneCli, enderecoCli, cpfCli, numeroCli, bairroCli,
        xMunCli, cMunCli, ufCli, cepCli, ieCli;
    int indIeCLi = 0;
    bool ehPfCli = false;
    QMap<QString, QString> fiscalValues;
    QMap<QString, QString> empresaValues;
    bool emitTodosNf = false;
    QString cStat, xMotivo, msg, nProt;
    bool existeItensComNcmVazio(QList<QList<QVariant> > listaProdutos, bool somenteNf);
    bool existeProdutosComNF(QList<QList<QVariant> > listaProdutos);
    QString enviarNfce(NfceACBR *nfce);
    void salvarNfceBD(NfceACBR *nfce);
    QString enviarNfe(NfeACBR *nfe);
    void salvarNfeBD(NfeACBR *nfe);
signals:
    void gerarEnviarNf();


protected:

};

#endif // PAGAMENTOVENDA_H
