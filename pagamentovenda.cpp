#include "pagamentovenda.h"

pagamentoVenda::pagamentoVenda(venda *ptrVenda, QString total, QString cliente, QString data, QWidget *parent)
    : pagamento(total, cliente, data, parent)
{
    janelaVenda = ptrVenda;
}
