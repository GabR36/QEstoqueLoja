#ifndef VENDA_H
#define VENDA_H

#include <QDialog>
#include <QVector>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include "vendas.h"
#include <QItemSelection>
#include "mainwindow.h"
#include <QLocale>

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
    QLocale portugues;
    QIcon deletar;


protected:
    void handleSelectionChangeProdutos(const QItemSelection &selected, const QItemSelection &deselected);

    void keyPressEvent(QKeyEvent *event) override;
private slots:
    void on_Btn_SelecionarProduto_clicked();

    void handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected);

    void on_Btn_Pesquisa_clicked();

    void on_Btn_Aceitar_clicked();

    void on_Ledit_Pesquisa_textChanged(const QString &arg1);

    void on_Tview_ProdutosSelecionados_customContextMenuRequested(const QPoint &pos);

    void deletarProd();

    void on_Ledit_Pesquisa_returnPressed();

    void on_Btn_CancelarVenda_clicked();

private:
    QSqlQueryModel *modeloProdutos = new QSqlQueryModel;
    QStandardItemModel *modeloSelecionados = new QStandardItemModel;
    Ui::venda *ui;
    QAction *actionMenuDeletarProd;


};

#endif // VENDA_H
