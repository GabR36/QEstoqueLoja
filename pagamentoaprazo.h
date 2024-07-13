#ifndef PAGAMENTOAPRAZO_H
#define PAGAMENTOAPRAZO_H

#include "pagamento.h"


class pagamentoAPrazo : public pagamento
{
    Q_OBJECT
public:
    explicit pagamentoAPrazo(QString total, QString cliente, QString data, QWidget *parent = nullptr);
};

#endif // PAGAMENTOAPRAZO_H
