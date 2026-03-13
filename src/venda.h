#ifndef VENDA_H
#define VENDA_H

#include <QVector>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include "vendas.h"
#include <QItemSelection>
#include "mainwindow.h"
#include <QLocale>
#include "dto/Config_dto.h"
#include "services/Produto_service.h"
#include "services/cliente_service.h"


namespace Ui {
class venda;
}

class venda : public QWidget
{
    Q_OBJECT

public:
    //Vendas *janelaVenda;
    QSqlDatabase db = QSqlDatabase::database();
    explicit venda(QWidget *parent = nullptr);
    ~venda();
    QString Total();
    QLocale portugues;
    QIcon deletar;



protected:
    void handleSelectionChangeProdutos(const QItemSelection &selected, const QItemSelection &deselected);

    void keyPressEvent(QKeyEvent *event) override;

    void focusInEvent(QFocusEvent *event) override;
    qlonglong validarCliente(bool mostrarMensagens);
    void atualizarListaCliente();
    void atualizarTotalProduto();
    void selecionarClienteNovo();
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

    void on_Btn_NovoCliente_clicked();

private:
    QSqlQueryModel *modeloProdutos = new QSqlQueryModel;
    QStandardItemModel *modeloSelecionados = new QStandardItemModel;
    Ui::venda *ui;
    QAction *actionMenuDeletarProd;
    QStringList clientesComId;
    ConfigDTO configDTO;
    QString getIdProdSelected();
    void verProd();
    Produto_Service prodServ;
    Cliente_service cliServ;
    QList<ProdutoVendidoDTO> obterProdutosSelecionados();
signals:
    void vendaConcluida();


};

#endif // VENDA_H
