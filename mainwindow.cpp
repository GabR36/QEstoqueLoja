#include "mainwindow.h"
#include "./ui_mainwindow.h"

QString nomeProduto;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_Btn_Enviar_clicked()
{
    nomeProduto += ui->Ledit_Nome->text() + " ";
    ui->TxtB_Info->setText(nomeProduto);
}

