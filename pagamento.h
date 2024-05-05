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
    explicit pagamento(QString total, QWidget *parent = nullptr);
    ~pagamento();
    venda *janelaVenda;
    QString cliente;
    QString data;
    QList<QList<QVariant>> rowDataList;
    QString totalGlobal;

private slots:
    void on_buttonBox_accepted();

    void on_Ledit_Recebido_textChanged(const QString &arg1);

private:
    Ui::pagamento *ui;
};

#endif // PAGAMENTO_H
