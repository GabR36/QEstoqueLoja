#ifndef VENDAS_H
#define VENDAS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QItemSelection>
#include "mainwindow.h"
#include <QLocale>

namespace Ui {
class Vendas;
}

class Vendas : public QWidget
{
    Q_OBJECT

public:
    MainWindow *janelaPrincipal;
    QSqlDatabase db = QSqlDatabase::database();
    explicit Vendas(QWidget *parent = nullptr);
    ~Vendas();
    void atualizarTabelas();
    QLocale portugues;

private slots:
    void on_Btn_InserirVenda_clicked();

    void handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected);

    void on_Btn_DeletarVenda_clicked();

    void on_DateEdt_De_dateChanged(const QDate &date);

    void on_DateEdt_Ate_dateChanged(const QDate &date);

private:
    void LabelLucro();
    void LabelLucro(QString whereQuery);
    QSqlQueryModel *modeloProdVendidos = new QSqlQueryModel;
    QSqlQueryModel *modeloVendas2 = new QSqlQueryModel;
    Ui::Vendas *ui;
    void filtrarData(QString de, QString ate);
};

#endif // VENDAS_H
