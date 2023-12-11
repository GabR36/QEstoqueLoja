#ifndef VENDA_H
#define VENDA_H

#include <QDialog>
#include <vector>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include "vendas.h"
#include <QItemSelection>

namespace Ui {
class venda;
}

class venda : public QDialog
{
    Q_OBJECT

public:
    Vendas *janelaVenda;
    QSqlDatabase db = QSqlDatabase::database();
    explicit venda(QWidget *parent = nullptr);
    ~venda();

private slots:
    void on_Btn_SelecionarProduto_clicked();

    void on_BtnBox_Venda_accepted();

    void handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QStandardItemModel modeloSelecionados;
    std::vector<std::pair<QString, QString>> vetorIds;
    Ui::venda *ui;
};

#endif // VENDA_H
