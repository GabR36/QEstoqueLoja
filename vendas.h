#ifndef VENDAS_H
#define VENDAS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QItemSelection>
#include "mainwindow.h"

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

private slots:
    void on_Btn_InserirVenda_clicked();
    void handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected);

    void on_CBox_Mes_activated(int index);

    void on_CBox_Dia_activated(int index);

    void on_CBox_Ano_activated(int index);

    void on_Btn_DeletarVenda_clicked();

    void on_Btn_AlterarVenda_clicked();

private:
    void LabelLucro();
    void LabelLucro(QString whereQuery);
    void queryCBox (int indexAno, int indexMes, int indexDia);
    QMap<QString, int> mapaMeses;
    QSqlQueryModel *modeloProdVendidos = new QSqlQueryModel;
    QSqlQueryModel *modeloVendas2 = new QSqlQueryModel;
    Ui::Vendas *ui;
};

#endif // VENDAS_H
