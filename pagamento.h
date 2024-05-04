#ifndef PAGAMENTO_H
#define PAGAMENTO_H

#include <QDialog>
#include "venda.h"

namespace Ui {
class pagamento;
}

class pagamento : public QDialog
{
    Q_OBJECT

public:
    explicit pagamento(QWidget *parent = nullptr);
    ~pagamento();
    venda *janelaVenda;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::pagamento *ui;
};

#endif // PAGAMENTO_H
