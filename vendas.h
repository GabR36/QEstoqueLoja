#ifndef VENDAS_H
#define VENDAS_H

#include <QWidget>

namespace Ui {
class Vendas;
}

class Vendas : public QWidget
{
    Q_OBJECT

public:
    explicit Vendas(QWidget *parent = nullptr);
    ~Vendas();

private slots:
    void on_Btn_InserirVenda_clicked();

private:
    Ui::Vendas *ui;
};

#endif // VENDAS_H
