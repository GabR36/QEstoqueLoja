#ifndef VENDAS_H
#define VENDAS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>

namespace Ui {
class Vendas;
}

class Vendas : public QWidget
{
    Q_OBJECT

public:
    QSqlDatabase db2 = QSqlDatabase::addDatabase("QSQLITE");
    QSqlDatabase db;
    explicit Vendas(QWidget *parent = nullptr);
    ~Vendas();
    void atualizarTabelas();

private slots:
    void on_Btn_InserirVenda_clicked();

private:
    QSqlQueryModel *modeloProdVendidos = new QSqlQueryModel;
    QSqlQueryModel *modeloVendas2 = new QSqlQueryModel;
    Ui::Vendas *ui;
};

#endif // VENDAS_H
