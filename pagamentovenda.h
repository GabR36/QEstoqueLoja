#ifndef PAGAMENTOVENDA_H
#define PAGAMENTOVENDA_H

#include "pagamento.h"
#include "venda.h"
#include "nota/nfcevenda.h"
#include "nota/nfevenda.h"
#include <CppBrasil/NFe/CppNFe>
#include <CppBrasil/DanfeQtRPT/CppDanfeQtRPT>
#include <qtrpt.h>
#include "subclass/waitdialog.h"
#include <qmap.h>
#include "nota/nfceacbr.h"

class pagamentoVenda : public pagamento
{
    Q_OBJECT
public:
    explicit pagamentoVenda(QList<QList<QVariant>> listaProdutos, QString total, QString cliente, QString data, int idCliente, QWidget *parent = nullptr);
    QList<QList<QVariant>> rowDataList;
    void imprimirDANFE(const CppNFe *cppnfe);
private slots:
    void on_CBox_ModeloEmit_currentIndexChanged(int index) override;
private:
    void terminarPagamento() override;
    int idCliente;
    NfceACBR *nfce;
    NfceVenda notaNFCe;
    NFeVenda notaNFe;
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
    bool existeItensComNcmVazio(QList<QList<QVariant> > listaProdutos, bool somenteNf);
    bool existeProdutosComNF(QList<QList<QVariant> > listaProdutos);
signals:
    void gerarEnviarNf();


protected:
    void onRetWSChange(const QString &webServices);
    void onErrorOccurred(const QString &error);
    void verificarErroNf(const CppNFe *cppnfe);
    void onRetStatusServico(const QString &status);
    void onRetLote(const QString &lote);
};

#endif // PAGAMENTOVENDA_H
