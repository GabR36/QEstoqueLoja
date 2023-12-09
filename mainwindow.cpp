#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "produto.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>
#include "alterarproduto.h"
#include "QItemSelectionModel"
#include "vender.h"
#include <qsqltablemodel.h>
#include "vendas.h"

QSqlTableModel *vendasModel;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // criar banco de dados e tabela se não foi ainda.
    db.setDatabaseName("estoque.db");
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados.";
    }
    QSqlQuery query;
    query.exec("CREATE TABLE produtos (id INTEGER PRIMARY KEY AUTOINCREMENT, quantidade INTEGER, descricao TEXT, preco DECIMAL(10,2))");
    if (query.isActive()) {
        qDebug() << "Tabela criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela: ";
    }
    query.exec("CREATE TABLE vendas (id INTEGER PRIMARY KEY AUTOINCREMENT, produto_id INTEGER, quantidade INTEGER, data_hora DATETIME DEFAULT CURRENT_TIMESTAMP)");
    if (query.isActive()) {
        qDebug() << "Tabela de vendas criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela de vendas: ";
    }
    query.exec("CREATE TABLE vendas2 (id INTEGER PRIMARY KEY AUTOINCREMENT, cliente TEXT, data_hora DATETIME DEFAULT CURRENT_TIMESTAMP)");
    if (query.isActive()) {
        qDebug() << "Tabela de vendas2 criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela de vendas2: ";
    }
    query.exec("CREATE TABLE produtos_vendidos (id INTEGER PRIMARY KEY AUTOINCREMENT, id_produto INTEGER, id_venda INTEGER, quantidade INTEGER, FOREIGN KEY (id_produto) REFERENCES produtos(id), FOREIGN KEY (id_venda) REFERENCES vendas2(id))");
    if (query.isActive()) {
        qDebug() << "Tabela de produtos_vendidos criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela de produtos_vendidos: ";
    }
    qDebug() << db.tables();





    // mostrar na tabela da aplicaçao a tabela do banco de dados.
    atualizarTableview();
    QSqlDatabase::database().close();
    //
    ui->Ledit_Desc->setFocus();
    // Selecionar a primeira linha da tabela
    QModelIndex firstIndex = model->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);

    QSqlTableModel *vendasModel;
    vendasModel = new QSqlTableModel(this);
    vendasModel->setTable("vendas");
    vendasModel->select(); // Carrega os dados da tabela de vendas

    // Configure o QTableView para exibir as vendas
    ui->tableViewVendas->setModel(vendasModel);
    ui->tableViewVendas->setEditTriggers(QAbstractItemView::NoEditTriggers); // Impede a edição das células
    ui->tableViewVendas->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Expande colunas para ajustar a largura









}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::atualizarTableview(){
    model->setQuery("SELECT * FROM produtos");
    ui->Tview_Produtos->setModel(model);



//    model->setQuery("SELECT * FROM vendas");
//    ui->tableViewVendas->setModel(model);
}

void MainWindow::on_Btn_Enviar_clicked()
{
    QString quantidadeProduto, descProduto, precoProduto;
    quantidadeProduto = ui->Ledit_Quantidade->text();
    descProduto = ui->Ledit_Desc->text();
    precoProduto = ui->Ledit_Preco->text();

    // adicionar ao banco de dados
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao enviar.";
    }
    QSqlQuery query;

    query.prepare("INSERT INTO produtos (quantidade, descricao, preco) VALUES (:valor1, :valor2, :valor3)");
    query.bindValue(":valor1", quantidadeProduto);
    query.bindValue(":valor2", descProduto);
    query.bindValue(":valor3", precoProduto);
    if (query.exec()) {
        qDebug() << "Inserção bem-sucedida!";
    } else {
        qDebug() << "Erro na inserção: ";
    }
    atualizarTableview();
    QSqlDatabase::database().close();
    // limpar campos para nova inserçao
    ui->Ledit_Desc->clear();
    ui->Ledit_Quantidade->clear();
    ui->Ledit_Preco->clear();
    ui->Ledit_Desc->setFocus();
}



void MainWindow::on_Btn_Delete_clicked()
{
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QString productId = idVariant.toString();

    // remover registro do banco de dados
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao deletar.";
    }
    QSqlQuery query;

    query.prepare("DELETE FROM produtos WHERE id = :valor1");
    query.bindValue(":valor1", productId);
    if (query.exec()) {
        qDebug() << "Delete bem-sucedido!";
    } else {
        qDebug() << "Erro no Delete: ";
    }
    atualizarTableview();
    QSqlDatabase::database().close();
}


void MainWindow::on_Btn_Pesquisa_clicked()
{
    QString pesquisa = ui->Ledit_Pesquisa->text();
    // mostrar na tableview a consulta
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao pesquisar.";
    }
    model->setQuery("SELECT * FROM produtos WHERE descricao LIKE '%" + pesquisa + "%'");
    ui->Tview_Produtos->setModel(model);
    QSqlDatabase::database().close();
}


void MainWindow::on_Btn_Vender_clicked()
{
    QString idVenda = ui->Ledit_VendaId->text();
    QString quantVenda = ui->Ledit_VendaQuant->text();
    compravenda(idVenda, quantVenda, false);

    ui->tableViewVendas->setModel(vendasModel);






}


void MainWindow::on_Btn_Comprar_clicked()
{
    QString idVenda = ui->Ledit_VendaId->text();
    QString quantVenda = ui->Ledit_VendaQuant->text();
    compravenda(idVenda, quantVenda, true);


}


void MainWindow::compravenda(QString idVenda, QString quantVenda, bool compravenda){
    QString maismenos;
    if (compravenda){
        maismenos = "+";
    } else {
        maismenos = "-";
    }
    // alterar banco de dados
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao comprar.";
    }
    QSqlQuery query;

    query.prepare("UPDATE produtos SET quantidade = quantidade " + maismenos + " :valor1 WHERE id = :valor2");
    query.bindValue(":valor1", quantVenda);
    query.bindValue(":valor2", idVenda);
    if (query.exec()) {
        qDebug() << "Inserção bem-sucedida!";
    } else {
        qDebug() << "Erro na inserção: ";
    }

    query.prepare("UPDATE produtos SET quantidade = quantidade " + maismenos + " :valor1 WHERE id = :valor2");
    query.bindValue(":valor1", quantVenda);
    query.bindValue(":valor2", idVenda);
    if (query.exec()) {
        qDebug() << "Atualização de quantidade bem-sucedida!";
    } else {
        qDebug() << "Erro na atualização de quantidade: ";
    }


    query.prepare("INSERT INTO vendas (produto_id, quantidade) VALUES (:valor1, :valor2)");
    query.bindValue(":valor1", idVenda);
    query.bindValue(":valor2", quantVenda);
    if (query.exec()) {
        qDebug() << "Venda registrada com sucesso!";
    } else {
        qDebug() << "Erro ao registrar venda: ";
    }

    // Atualizar a tabela de produtos
    query.prepare("UPDATE produtos SET quantidade = quantidade " + maismenos + " :valor1 WHERE id = :valor2");
    query.bindValue(":valor1", quantVenda);
    query.bindValue(":valor2", idVenda);
    if (query.exec()) {
        qDebug() << "Atualização de quantidade bem-sucedida!";
    } else {
        qDebug() << "Erro na atualização de quantidade: ";
    }
    // mostrar na tableview
    atualizarTableview();
    QSqlDatabase::database().close();
    // limpar campos para nova inserção
    ui->Ledit_VendaQuant->clear();
    ui->Ledit_VendaId->clear();
    //adsa
}


void MainWindow::on_Btn_Alterar_clicked()
{
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QVariant quantVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 1));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));
    QString productId = idVariant.toString();
    QString productQuant = quantVariant.toString();
    QString productDesc = descVariant.toString();
    QString productPreco = precoVariant.toString();
    qDebug() << productId;
    qDebug() << productPreco;
    // criar janela
    AlterarProduto *alterar = new AlterarProduto;
    alterar->janelaPrincipal = this;
    alterar->idAlt = productId;
    alterar->TrazerInfo(productDesc, productQuant, productPreco);
    alterar->show();
}


void MainWindow::on_Btn_Venda_clicked()
{
    Vendas *vendas = new Vendas;
    vendas->show();
}

