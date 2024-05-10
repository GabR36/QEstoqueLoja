#ifndef VENDA_H
#define VENDA_H

#include <QDialog>
#include <QVector>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include "vendas.h"
#include <QItemSelection>
#include "mainwindow.h"

namespace Ui {
class venda;
}

class venda : public QDialog
{
    Q_OBJECT

public:
    Vendas *janelaVenda;
    MainWindow *janelaPrincipal;
    QSqlDatabase db = QSqlDatabase::database();
    explicit venda(QWidget *parent = nullptr);
    ~venda();
    QString Total();


private slots:
    void on_Btn_SelecionarProduto_clicked();

    void handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected);

    void on_Btn_Pesquisa_clicked();

    void on_Ledit_QuantVendido_textChanged(const QString &arg1);

    void on_Ledit_Preco_textChanged(const QString &arg1);

    void on_Ledit_Barras_returnPressed();

    void on_Btn_Aceitar_clicked();

private:
    QSqlQueryModel *modeloProdutos = new QSqlQueryModel;
    QStandardItemModel modeloSelecionados;
    Ui::venda *ui;
};

#endif // VENDA_H
