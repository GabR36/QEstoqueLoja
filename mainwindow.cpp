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
    QSqlQuery query;
    query.exec("CREATE TABLE produtos (id INTEGER PRIMARY KEY AUTOINCREMENT, nome TEXT, quantidade INTEGER, descricao TEXT)");
    if (query.isActive()) {
        qDebug() << "Tabela criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela: ";
    }
    qDebug() << db.tables();

    // mostrar na tabela da aplicaçao a tabela do banco de dados.
    QSqlQueryModel* model = new QSqlQueryModel;
    model->setQuery("SELECT * FROM produtos");
    ui->Tview_Produtos->setModel(model);
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
    // adicionar ao banco de dados
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao enviar.";
    }
    QSqlQuery query;

    query.prepare("INSERT INTO produtos (nome, quantidade, descricao) VALUES (:valor1, :valor2, :valor3)");
    query.bindValue(":valor1", nomeProduto);
    query.bindValue(":valor2", quantidadeProduto);
    query.bindValue(":valor3", descProduto);
    if (query.exec()) {
        qDebug() << "Inserção bem-sucedida!";
    } else {
        qDebug() << "Erro na inserção: ";
    }
    QSqlDatabase::database().close();
}

