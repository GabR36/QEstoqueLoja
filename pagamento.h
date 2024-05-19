#ifndef PAGAMENTO_H
#define PAGAMENTO_H

#include <QDialog>
#include "venda.h"
#include <QLocale>

namespace Ui {
class pagamento;
}

class pagamento : public QDialog
{
    Q_OBJECT

public:
    explicit pagamento(QString total, QString cliente, QString data, QWidget *parent = nullptr);
    ~pagamento();
    venda *janelaVenda;
    QString clienteGlobal;
    QString dataGlobal;
    QList<QList<QVariant>> rowDataList;
    QString totalGlobal;
    QLocale portugues;

private slots:
    void on_buttonBox_accepted();

    void on_Ledit_Recebido_textChanged(const QString &arg1);

    void on_CBox_FormaPagamento_activated(int index);

    void on_Ledit_Taxa_textChanged(const QString &arg1);

private:
    Ui::pagamento *ui;
};

#endif // PAGAMENTO_H
