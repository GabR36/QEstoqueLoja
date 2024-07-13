#include "pagamentoaprazo.h"

pagamentoAPrazo::pagamentoAPrazo(QString total, QString cliente, QString data, QWidget *parent)
    : pagamento(total, cliente, data, parent)
{
    ui->Ledit_Desconto->setText("teste");
}
