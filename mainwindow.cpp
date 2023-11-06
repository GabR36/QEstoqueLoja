#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "produto.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>
#include "vender.h"



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
    query.exec("CREATE TABLE produtos (id INTEGER PRIMARY KEY AUTOINCREMENT, nome TEXT, quantidade INTEGER, descricao TEXT, preco DECIMAL(10,2))");
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
    qDebug() << db.tables();





    // mostrar na tabela da aplicaçao a tabela do banco de dados.
    atualizarTableview();
    QSqlDatabase::database().close();
    //
    ui->Ledit_Nome->setFocus();

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


   QSqlTableModel *vendasModel;
   vendasModel = new QSqlTableModel(this);
   vendasModel->setTable("vendas");
   vendasModel->select(); // Carrega os dados da tabela de vendas

    // Configure o QTableView para exibir as vendas
    ui->tableViewVendas->setModel(vendasModel);
    ui->tableViewVendas->setEditTriggers(QAbstractItemView::NoEditTriggers); // Impede a edição das células
    ui->tableViewVendas->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Expande colunas para ajustar a largura








}

void MainWindow::on_Btn_Enviar_clicked()
{
    QString nomeProduto, quantidadeProduto, descProduto, precoProduto;
    nomeProduto = ui->Ledit_Nome->text();
    quantidadeProduto = ui->Ledit_Quantidade->text();
    descProduto = ui->Ledit_Desc->text();
    precoProduto = ui->Ledit_Preco->text();

    // adicionar ao banco de dados
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao enviar.";
    }
    QSqlQuery query;

    query.prepare("INSERT INTO produtos (nome, quantidade, descricao, preco) VALUES (:valor1, :valor2, :valor3, :valor4)");
    query.bindValue(":valor1", nomeProduto);
    query.bindValue(":valor2", quantidadeProduto);
    query.bindValue(":valor3", descProduto);
    query.bindValue(":valor4", precoProduto);
    if (query.exec()) {
        qDebug() << "Inserção bem-sucedida!";
    } else {
        qDebug() << "Erro na inserção: ";
    }
    atualizarTableview();
    QSqlDatabase::database().close();
    // limpar campos para nova inserçao
    ui->Ledit_Desc->clear();
    ui->Ledit_Nome->clear();
    ui->Ledit_Quantidade->clear();
    ui->Ledit_Preco->clear();
    ui->Ledit_Nome->setFocus();
}



void MainWindow::on_Btn_Delete_clicked()
{
    QString idDelet = ui->Ledit_Delete->text();
    // remover registro do banco de dados
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao deletar.";
    }
    QSqlQuery query;

    query.prepare("DELETE FROM produtos WHERE id = :valor1");
    query.bindValue(":valor1", idDelet);
    if (query.exec()) {
        qDebug() << "Delete bem-sucedido!";
    } else {
        qDebug() << "Erro no Delete: ";
    }
    atualizarTableview();
    QSqlDatabase::database().close();
    // limpar campo para nova inserçao
    ui->Ledit_Delete->clear();
    ui->Ledit_Delete->setFocus();
}


void MainWindow::on_Btn_Pesquisa_clicked()
{
    QString pesquisa = ui->Ledit_Pesquisa->text();
    // mostrar na tableview a consulta
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao pesquisar.";
    }
    model->setQuery("SELECT * FROM produtos WHERE nome LIKE '%" + pesquisa + "%'");
    ui->Tview_Produtos->setModel(model);
    QSqlDatabase::database().close();
}


void MainWindow::on_Btn_Vender_clicked()
{
    QString idVenda = ui->Ledit_VendaId->text();
    QString quantVenda = ui->Ledit_VendaQuant->text();
    compravenda(idVenda, quantVenda, false);














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
        qDebug() << "Atualização de quantidade bem-sucedida!";
    } else {
        qDebug() << "Erro na atualização de quantidade: ";
    }

//--------------------------
    query.prepare("INSERT INTO vendas (produto_id, quantidade) VALUES (:valor1, :valor2)");
    query.bindValue(":valor1", idVenda);
    query.bindValue(":valor2", quantVenda);
    if (query.exec()) {
        qDebug() << "Venda registrada com sucesso!";
    } else {
        qDebug() << "Erro ao registrar venda: ";
    }



    // mostrar na tableview
    atualizarTableview();

    QSqlDatabase::database().close();
    // limpar campos para nova inserção
    ui->Ledit_VendaQuant->clear();
    ui->Ledit_VendaId->clear();
    //adsa
}






