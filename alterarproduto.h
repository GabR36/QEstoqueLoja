#ifndef ALTERARPRODUTO_H
#define ALTERARPRODUTO_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class AlterarProduto;
}

class AlterarProduto : public QDialog
{
    Q_OBJECT

public:
    MainWindow *janelaPrincipal;
    QString idAlt;
    explicit AlterarProduto(QWidget *parent = nullptr);
    ~AlterarProduto();
    void TrazerInfo(QString nome, QString desc, QString quant, QString preco);

private slots:
    void on_Btn_AltAceitar_accepted();

private:
    Ui::AlterarProduto *ui;
};

#endif // ALTERARPRODUTO_H
