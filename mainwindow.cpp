#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

QString nomeProduto;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile arquivo("../QEstoqueLoja/estoque.txt");
    if (arquivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream entrada(&arquivo);
        nomeProduto = entrada.readAll();
        ui->TxtB_Info->setPlainText(nomeProduto);
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
    nomeProduto += ui->Ledit_Nome->text() + "\n";
    ui->TxtB_Info->setText(nomeProduto);
    QFile arquivo("../QEstoqueLoja/estoque.txt");
    if (arquivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // Crie um objeto QTextStream para escrever no arquivo
        QTextStream out(&arquivo);
        // Escreva os dados no arquivo
        out << nomeProduto;
        // Feche o arquivo
        arquivo.flush();
        arquivo.close();
    } else {
        QMessageBox::warning(this,"ERRO", "Algo deu errado ao escrever no arquivo.");
    }
}

