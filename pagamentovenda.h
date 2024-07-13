#ifndef PAGAMENTOVENDA_H
#define PAGAMENTOVENDA_H

#include "pagamento.h"
#include "venda.h"

class pagamentoVenda : public pagamento
{
    Q_OBJECT
public:
    pagamentoVenda(venda *ptrVenda, QString total, QString cliente, QString data, QWidget *parent);
    venda *janelaVenda;
};

#endif // PAGAMENTOVENDA_H
