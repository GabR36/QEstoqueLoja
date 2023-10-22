#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
// #include <QHeaderView>
#include <QStandardItemModel>
#include "produto.h"

QString nomeProduto, quantidadeProduto, registro, descProduto;
int rowCount;
QStandardItemModel *model = new QStandardItemModel();

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile arquivo("../QEstoqueLoja/estoque.txt");
    if (arquivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream entrada(&arquivo);
        registro = entrada.readAll();
        ui->TxtB_Info->setPlainText(registro);
        arquivo.close();
    } else {
        QMessageBox::warning(this,"ERRO", "Algo deu errado ao abrir o arquivo.");
    }
    ui->Ledit_Nome->setFocus();
    // QHeaderView cabecalhoTabela(Qt::Horizontal, ui->widget);
    // ui->Tview_Produtos->setHorizontalHeader(cabecalhoTabela);
    model->setRowCount(3);
    model->setColumnCount(3);
    model->setHorizontalHeaderItem(0, new QStandardItem("Nome"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Quantidade"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Descrição"));

    QStandardItem *item1 = new QStandardItem("Alice");
    QStandardItem *item2 = new QStandardItem("25");

    model->setItem(0, 0, item1);
    model->setItem(0, 1, item2);
    ui->Tview_Produtos->setModel(model);
    // ui->Tview_Produtos->horizontalHeader();
    model->setRowCount(3+1);
    ui->Tview_Produtos->setModel(model);
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
    registro += nomeProduto + " " + quantidadeProduto + " " + descProduto + "\n";
    QStandardItem *newNome = new QStandardItem(nomeProduto);
    QStandardItem *newQuantidade = new QStandardItem(quantidadeProduto);
    QStandardItem *newDesc = new QStandardItem(descProduto);
    rowCount = model->rowCount();
    model->setItem(rowCount, 0, newNome);
    model->setItem(rowCount, 1, newQuantidade);
    model->setItem(rowCount, 2, newDesc);
    ui->TxtB_Info->setText(registro);
    QFile arquivo("../QEstoqueLoja/estoque.txt");
    if (arquivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
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

