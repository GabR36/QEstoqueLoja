#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

QString nomeProduto, quantidadeProduto, registro;

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
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_Btn_Enviar_clicked()
{
    nomeProduto = ui->Ledit_Nome->text();
    quantidadeProduto = ui->Ledit_Quantidade->text();
    registro += nomeProduto + " " + quantidadeProduto + "\n";
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
    } else {
        QMessageBox::warning(this,"ERRO", "Algo deu errado ao escrever no arquivo.");
    }
}

