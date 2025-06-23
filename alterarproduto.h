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
    QString ucomAlt,precoFornAlt,porcentLucroAlt,ncmAlt,cestAlt,aliquotaImpAlt;
    bool nfAlt;
    explicit AlterarProduto(QWidget *parent = nullptr);
    ~AlterarProduto();
    QLocale portugues;

    void TrazerInfo(QString desc, QString quant, QString preco, QString barras, bool nf, QString ucom, QString precoforn, QString porcentlucro, QString ncm, QString cest, QString aliquotaimp);
private slots:
    void on_Btn_AltAceitar_accepted();

    void on_Btn_GerarCod_clicked();

    void on_Ledit_AltNCM_editingFinished();

    void on_Ledit_AltPrecoFornecedor_textChanged(const QString &arg1);

    void on_Ledit_AltPercentualLucro_textChanged(const QString &arg1);

    void on_Ledit_AltPreco_textChanged(const QString &arg1);

private:
    Ui::AlterarProduto *ui;
    QSet<QString> generatedNumbers;
    bool atualizando = false;
signals:
    void produtoAlterado();
};

#endif // ALTERARPRODUTO_H
