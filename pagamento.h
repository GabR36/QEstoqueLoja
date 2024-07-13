#ifndef PAGAMENTO_H
#define PAGAMENTO_H

#include <QDialog>
#include "venda.h"
#include "vendas.h"
#include <QLocale>
#include "ui_pagamento.h"
#include <QSqlDatabase>

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
    QSqlDatabase db = QSqlDatabase::database();

private slots:
    void on_buttonBox_accepted();

    void on_Ledit_Recebido_textChanged(const QString &arg1);

    void on_CBox_FormaPagamento_activated(int index);

    void on_Ledit_Taxa_textChanged(const QString &arg1);

    void on_Ledit_Desconto_textChanged(const QString &arg1);



protected:
    float obterValorFinal(QString taxa, QString desconto);
    void descontoTaxa();
    Ui::pagamento *ui;
};

#endif // PAGAMENTO_H
