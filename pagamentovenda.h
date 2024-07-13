#ifndef PAGAMENTOVENDA_H
#define PAGAMENTOVENDA_H

#include "pagamento.h"
#include "venda.h"

class pagamentoVenda : public pagamento
{
    Q_OBJECT
public:
    explicit pagamentoVenda(QList<QList<QVariant>> listaProdutos, venda *ptrVenda, QString total, QString cliente, QString data, QWidget *parent = nullptr);
    venda *janelaVenda;
    QList<QList<QVariant>> rowDataList;

private:
    void terminarPagamento() override;
};

#endif // PAGAMENTOVENDA_H
