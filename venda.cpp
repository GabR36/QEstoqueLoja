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
    db2.setDatabaseName("estoque.db");
    QSqlQueryModel *modeloProdutos = new QSqlQueryModel;
    modeloProdutos->setQuery("SELECT * FROM produtos");
    ui->Tview_Produtos->setModel(modeloProdutos);
    modeloSelecionados.setHorizontalHeaderItem(0, new QStandardItem("ID_Produto"));
    modeloSelecionados.setHorizontalHeaderItem(1, new QStandardItem("Quantidade_Vendida"));
    modeloSelecionados.setHorizontalHeaderItem(2, new QStandardItem("Descricao"));
    modeloSelecionados.setHorizontalHeaderItem(3, new QStandardItem("Preço"));
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
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));
    QString idProduto = idVariant.toString();
    QString descProduto = descVariant.toString();
    QString precoProduto = precoVariant.toString();
    vetorIds.push_back(std::make_pair(idProduto, quantVendido));
    ui->Ledit_QuantVendido->clear();
    qDebug() << vetorIds;
    // mostrar na tabela Selecionados
    modeloSelecionados.appendRow({new QStandardItem(idProduto), new QStandardItem(quantVendido), new QStandardItem(descProduto), new QStandardItem(precoProduto)});
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
}


void venda::on_BtnBox_Venda_accepted()
{
    QString cliente = ui->Ledit_Cliente->text();
    // adicionar ao banco de dados
    if(!db2.open()){
        qDebug() << "erro ao abrir banco de dados. botao aceitar venda.";
        return;
    }
    QSqlQuery query(db2);

    query.prepare("INSERT INTO vendas2 (cliente) VALUES (:valor1)");
    query.bindValue(":valor1", cliente);
    QString idVenda;
    if (query.exec()) {
        idVenda = query.lastInsertId().toString();
        qDebug() << "Inserção bem-sucedida!";
    } else {
        qDebug() << "Erro na inserção: ";
    }
    // adicionar ao banco de dados
    for (const auto& selecionado : vetorIds) {
        query.prepare("INSERT INTO produtos_vendidos (id_produto, quantidade, id_venda) VALUES (:valor1, :valor2, :valor3)");
        query.bindValue(":valor1", selecionado.first);
        query.bindValue(":valor2", selecionado.second);
        query.bindValue(":valor3", idVenda);
        if (query.exec()) {
            qDebug() << "Inserção prod_vendidos bem-sucedida!";
        } else {
            qDebug() << "Erro na inserção prod_vendidos: ";
        }
    }
    db2.close();
    janelaVenda->atualizarTabelas();
}

