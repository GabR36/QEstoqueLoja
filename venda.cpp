#include "venda.h"
#include "ui_venda.h"
#include <QSqlQueryModel>

venda::venda(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::venda)
{
    ui->setupUi(this);
    QSqlQueryModel *modeloProdutos = new QSqlQueryModel;
    modeloProdutos->setQuery("SELECT * FROM produtos");
    ui->Tview_Produtos->setModel(modeloProdutos);
}

venda::~venda()
{
    delete ui;
}

void venda::on_Btn_SelecionarProduto_clicked()
{
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QString idProduto = idVariant.toString();
    vetorIds.push_back(idProduto);
    QString idQuery;
    for (int i = 0; i < vetorIds.size(); i++){
        idQuery = idQuery + " id = " + vetorIds[i] + " OR";
    }
    int ultimoEspaço = idQuery.lastIndexOf(' ');
    idQuery = idQuery.left(ultimoEspaço);
    QSqlQueryModel *modeloSelecionados = new QSqlQueryModel;
    modeloSelecionados->setQuery("SELECT * FROM produtos WHERE" + idQuery);
    ui->Tview_ProdutosSelecionados->setModel(modeloSelecionados);
    qDebug() << idQuery;
    qDebug() << vetorIds;
}

