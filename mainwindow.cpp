#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "produto.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>

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

    // verificar se tabela ja existe
    QSqlQuery query;
    query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='produtos'");

    if (query.next()) {
        qDebug() << "a tabela produtos já existe.";
    }
    else {
        QSqlQuery query;
        query.exec("CREATE TABLE produtos (id INTEGER PRIMARY KEY AUTOINCREMENT, nome TEXT, quantidade INTEGER, descricao TEXT)");
        if (query.isActive()) {
            qDebug() << "Tabela criada com sucesso!";
        } else {
            qDebug() << "Erro ao criar tabela: ";
        }
    }
    qDebug() << db.tables();

    // mostrar na tabela da aplicaçao a tabela do banco de dados.
    QSqlQueryModel* model = new QSqlQueryModel;
    model->setQuery("SELECT * FROM produtos");
    ui->Tview_Produtos->setModel(model);
    QSqlDatabase::database().close();
//    ui->Tview_Produtos->show();
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
//    Produto addProduto(nomeProduto, descProduto, quantidadeProduto.toInt());
//    produtos.push_back(addProduto);
//    registro = nomeProduto + "," + quantidadeProduto + "," + descProduto + "\n";
//    QStandardItem *newNome = new QStandardItem(nomeProduto);
//    QStandardItem *newQuantidade = new QStandardItem(quantidadeProduto);
//    QStandardItem *newDesc = new QStandardItem(descProduto);
//    rowCount = model->rowCount();
//    model->setItem(rowCount, 0, newNome);
//    model->setItem(rowCount, 1, newQuantidade);
//    model->setItem(rowCount, 2, newDesc);
//    QFile arquivo("../QEstoqueLoja/estoque.txt");
//    if (arquivo.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
//        // Crie um objeto QTextStream para escrever no arquivo
//        QTextStream out(&arquivo);
//        // Escreva os dados no arquivo
//        out << registro;
//        // Feche o arquivo
//        arquivo.flush();
//        arquivo.close();
//        ui->Ledit_Desc->clear();
//        ui->Ledit_Nome->clear();
//        ui->Ledit_Quantidade->clear();
//        ui->Ledit_Nome->setFocus();
//    } else {
//        QMessageBox::warning(this,"ERRO", "Algo deu errado ao escrever no arquivo.");
//    }
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao enviar.";
    }
    QSqlQuery query;

    // Definir o comando SQL de inserção
    QString insertSql = "INSERT INTO produtos (nome, quantidade, descricao) VALUES (:valor2, :valor3, :valor4)";
    query.prepare(insertSql);

    // Atribuir valores aos parâmetros
    query.bindValue(":valor2", nomeProduto);
    query.bindValue(":valor3", quantidadeProduto);
    query.bindValue(":valor4", descProduto);

    // Executar a consulta
    if (query.exec()) {
        qDebug() << "Inserção bem-sucedida!";
    } else {
        qDebug() << "Erro na inserção: ";
    }
}

