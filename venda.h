#ifndef VENDA_H
#define VENDA_H

#include <QDialog>
#include <vector>
#include <QSqlDatabase>
#include <QStandardItemModel>

namespace Ui {
class venda;
}

class venda : public QDialog
{
    Q_OBJECT

public:
    QSqlDatabase db;
    explicit venda(QWidget *parent = nullptr);
    ~venda();

private slots:
    void on_Btn_SelecionarProduto_clicked();

    void on_buttonBox_accepted();

private:
    QStandardItemModel modeloSelecionados;
    std::vector<std::pair<QString, QString>> vetorIds;
    Ui::venda *ui;
};

#endif // VENDA_H
