#ifndef VENDAS_H
#define VENDAS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QItemSelection>
#include "mainwindow.h"
#include <QLocale>
#include "entradasvendasprazo.h"


namespace Ui {
class Vendas;
}

class Vendas : public QWidget
{
    Q_OBJECT


public:
    MainWindow *janelaPrincipal;
    QSqlDatabase db = QSqlDatabase::database();
    explicit Vendas(QWidget *parent = nullptr, int idCliente = 0);
    ~Vendas();
    void atualizarTabelas();
    QLocale portugues;
    QList<QList<QVariant>> rowDataList;
    static bool imprimirReciboVenda( QString idVenda);
    static QStringList getProdutosVendidos( QString idVenda);
    void actionAbrirPagamentosVenda(QString id_venda);


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

    void on_cb_BuscaVendasPrazo_stateChanged(int arg1);

    void on_Btn_AbrirPag_clicked();

    void on_Tview_ProdutosVendidos_customContextMenuRequested(const QPoint &pos);

private:
    // void LabelLucro();
    void LabelLucro(QString whereQueryData, QString whereQueryPrazo);
    QSqlQueryModel *modeloProdVendidos = new QSqlQueryModel;
    QSqlQueryModel *modeloVendas2 = new QSqlQueryModel;
    Ui::Vendas *ui;
    void filtrarData(QString de1, QString ate1);
    QAction *actionMenuDeletarVenda;
    QAction *actionImprimirRecibo;
    QAction *actionAbrirPagamentos;
    QAction *actionMenuDevolverProd;
    QString de, ate;
    void Teste();
    QString idVendaSelec;
    void devolverProdutoVenda(QString id_venda, QString id_prod_vend);
    void devolverProduto(QString id_prod_vend, QString id_produto, QString qntd);
    void mostrarVendasCliente(int idCliente);
    int IDCLIENTE;
};

#endif // VENDAS_H
