#ifndef VENDA_H
#define VENDA_H

#include <QDialog>

namespace Ui {
class venda;
}

class venda : public QDialog
{
    Q_OBJECT

public:
    explicit venda(QWidget *parent = nullptr);
    ~venda();

private slots:
    void on_Btn_SelecionarProduto_clicked();

private:
    Ui::venda *ui;
};

#endif // VENDA_H
