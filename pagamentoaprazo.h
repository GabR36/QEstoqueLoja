#ifndef PAGAMENTOAPRAZO_H
#define PAGAMENTOAPRAZO_H

#include "pagamento.h"
#include "entradasvendasprazo.h"



class pagamentoAPrazo : public pagamento
{
    Q_OBJECT
public:
    explicit pagamentoAPrazo(QString id_venda, QString total, QString cliente, QString data, QWidget *parent = nullptr);
    virtual ~pagamentoAPrazo();
protected:
    void terminarPagamento() override;
    QString idVenda;
signals:
    void pagamentoPrazoConcluido();
};

#endif // PAGAMENTOAPRAZO_H
