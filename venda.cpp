#include "venda.h"
#include "ui_venda.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QStandardItemModel>

venda::venda(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::venda)
{
    ui->setupUi(this);
    QSqlQueryModel *modeloProdutos = new QSqlQueryModel;
    modeloProdutos->setQuery("SELECT * FROM produtos");
    ui->Tview_Produtos->setModel(modeloProdutos);
    modeloSelecionados.setHorizontalHeaderItem(0, new QStandardItem("ID_Produto"));
    modeloSelecionados.setHorizontalHeaderItem(1, new QStandardItem("Quantidade_Vendida"));
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
}

venda::~venda()
{
    delete ui;
}

void venda::on_Btn_SelecionarProduto_clicked()
{
    // pegar id do produto selecionado e quant do Ledit
    QString quantVendido = ui->Ledit_QuantVendido->text();
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QString idProduto = idVariant.toString();
    vetorIds.push_back(std::make_pair(idProduto, quantVendido));
    ui->Ledit_QuantVendido->clear();
    qDebug() << vetorIds;
    // mostrar na tabela Selecionados
    modeloSelecionados.appendRow({new QStandardItem(idProduto), new QStandardItem(quantVendido)});
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
}


void venda::on_buttonBox_accepted()
{
//    QString cliente = ui->Ledit_Cliente->text();
//    // adicionar ao banco de dados
//    if(!db.open()){
//        qDebug() << "erro ao abrir banco de dados. botao enviar.";
//    }
//    QSqlQuery query;

//    query.prepare("INSERT INTO vendas2 (cliente) VALUES (:valor1)");
//    query.bindValue(":valor1", cliente);

//    if (query.exec()) {
//        qDebug() << "Inserção bem-sucedida!";
//    } else {
//        qDebug() << "Erro na inserção: ";
//    }

//    query.prepare("INSERT INTO produtos_vendidos ");
//    QSqlDatabase::database().close();
}

