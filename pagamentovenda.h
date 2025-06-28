#ifndef PAGAMENTOVENDA_H
#define PAGAMENTOVENDA_H

#include "pagamento.h"
#include "venda.h"
#include "nota/nfcevenda.h"
#include <CppBrasil/NFe/CppNFe>
#include <CppBrasil/DanfeQtRPT/CppDanfeQtRPT>
#include <qtrpt.h>
#include "subclass/waitdialog.h"
#include <qmap.h>


class pagamentoVenda : public pagamento
{
    Q_OBJECT
public:
    explicit pagamentoVenda(QList<QList<QVariant>> listaProdutos, QString total, QString cliente, QString data, int idCliente, QWidget *parent = nullptr);
    QList<QList<QVariant>> rowDataList;
    void imprimirDANFE(const CppNFe *cppnfe);
private:
    void terminarPagamento() override;
    int idCliente;
    NfceVenda nota;
    WaitDialog* waitDialog = nullptr;
    QString erroNf;
    QString idVenda;
    bool ehPfCliente = false;
    QMap<QString, QString> fiscalValues;
    QMap<QString, QString> empresaValues;

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
