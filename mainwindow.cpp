#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "produto.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile arquivo("../QEstoqueLoja/estoque.txt");
    if (arquivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream entrada(&arquivo);
        while(!entrada.atEnd()){
            QString linha = entrada.readLine();
            QStringList elementos = linha.split(',');
            if (elementos.size() == 3){
                QString nome = elementos[0];
                int quant = elementos[1].toInt();
                QString desc = elementos[2];
                Produto novoProduto(nome, desc, quant);
                produtos.push_back(novoProduto);
            }
            else{
                qDebug() << "erro.";
            }
        }
        arquivo.close();
    }
    else {
        QMessageBox::warning(this,"ERRO", "Algo deu errado ao abrir o arquivo.");
    }
    ui->Ledit_Nome->setFocus();
    model->setHorizontalHeaderItem(0, new QStandardItem("Nome"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Quantidade"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Descrição"));
    ui->Tview_Produtos->setModel(model);
    ui->Tview_Produtos->setColumnWidth(2, 165);

    for(int i = 0;i < produtos.size(); i++){
        QStandardItem *newNome = new QStandardItem(produtos[i].nome);
        QStandardItem *newQuantidade = new QStandardItem(QString::number(produtos[i].quantidade));
        QStandardItem *newDesc = new QStandardItem(produtos[i].descricao);
        rowCount = model->rowCount();
        model->setItem(rowCount, 0, newNome);
        model->setItem(rowCount, 1, newQuantidade);
        model->setItem(rowCount, 2, newDesc);
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("estoque.db");
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados.";
    }
    QSqlQuery query;
    query.exec("CREATE TABLE produtos (id INTEGER PRIMARY KEY, nome TEXT, quantidade INTEGER, descricao TEXT)");
    if (query.isActive()) {
        qDebug() << "Tabela criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela: ";
    }
    qDebug() << db.tables();
    QSqlDatabase::database().close();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_Btn_Enviar_clicked()
{
    nomeProduto = ui->Ledit_Nome->text();
    quantidadeProduto = ui->Ledit_Quantidade->text();
    descProduto = ui->Ledit_Desc->text();
    Produto addProduto(nomeProduto, descProduto, quantidadeProduto.toInt());
    produtos.push_back(addProduto);
    registro = nomeProduto + "," + quantidadeProduto + "," + descProduto + "\n";
    QStandardItem *newNome = new QStandardItem(nomeProduto);
    QStandardItem *newQuantidade = new QStandardItem(quantidadeProduto);
    QStandardItem *newDesc = new QStandardItem(descProduto);
    rowCount = model->rowCount();
    model->setItem(rowCount, 0, newNome);
    model->setItem(rowCount, 1, newQuantidade);
    model->setItem(rowCount, 2, newDesc);
    QFile arquivo("../QEstoqueLoja/estoque.txt");
    if (arquivo.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        // Crie um objeto QTextStream para escrever no arquivo
        QTextStream out(&arquivo);
        // Escreva os dados no arquivo
        out << registro;
        // Feche o arquivo
        arquivo.flush();
        arquivo.close();
        ui->Ledit_Desc->clear();
        ui->Ledit_Nome->clear();
        ui->Ledit_Quantidade->clear();
        ui->Ledit_Nome->setFocus();
    } else {
        QMessageBox::warning(this,"ERRO", "Algo deu errado ao escrever no arquivo.");
    }
}

