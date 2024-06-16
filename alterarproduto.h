#ifndef ALTERARPRODUTO_H
#define ALTERARPRODUTO_H

#include <QDialog>
#include "mainwindow.h"
#include <QLocale>
#include <QSet>

namespace Ui {
class AlterarProduto;
}

class AlterarProduto : public QDialog
{
    Q_OBJECT

public:
    MainWindow *janelaPrincipal;
    QString idAlt;
    QString descAlt;
    QString precoAlt;
    QString quantAlt;
    QString barrasAlt;
    bool nfAlt;
    explicit AlterarProduto(QWidget *parent = nullptr);
    ~AlterarProduto();
    void TrazerInfo(QString desc, QString quant, QString preco, QString barras, bool nf);
    QLocale portugues;

private slots:
    void on_Btn_AltAceitar_accepted();

    void on_Btn_GerarCod_clicked();

private:
    Ui::AlterarProduto *ui;
    QSet<QString> generatedNumbers;
};

#endif // ALTERARPRODUTO_H
