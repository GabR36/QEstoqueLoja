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
    QList<QList<QVariant>> rowDataList;
    static bool imprimirReciboVenda( QString idVenda);
    static QStringList getProdutosVendidos( QString idVenda);


public slots:
    void imprimirReciboVendaSelec(QString id); //precisa ser slot :(

private slots:
    void on_Btn_InserirVenda_clicked();

    void handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected);

    void on_Btn_DeletarVenda_clicked();

    void on_DateEdt_De_dateChanged(const QDate &date);

    void on_DateEdt_Ate_dateChanged(const QDate &date);

    void on_Tview_Vendas2_customContextMenuRequested(const QPoint &pos);

    void on_testebutton_clicked();

private:
    void LabelLucro();
    void LabelLucro(QString whereQuery);
    QSqlQueryModel *modeloProdVendidos = new QSqlQueryModel;
    QSqlQueryModel *modeloVendas2 = new QSqlQueryModel;
    Ui::Vendas *ui;
    void filtrarData(QString de, QString ate);
    QAction *actionMenuDeletarVenda;
    QAction *actionImprimirRecibo;
    void Teste();


};

#endif // VENDAS_H
