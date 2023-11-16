#include "alterarproduto.h"
#include "ui_alterarproduto.h"

AlterarProduto::AlterarProduto(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlterarProduto)
{
    ui->setupUi(this);
    ui->Ledit_AltNome->setText("teste");
}

AlterarProduto::~AlterarProduto()
{
    delete ui;
}

 void AlterarProduto::TrazerInfo(QString nome, QString desc, QString quant, QString preco){
    ui->Ledit_AltNome->setText(nome);
    ui->Ledit_AltDesc->setText(desc);
    ui->Ledit_AltQuant->setText(quant);
    ui->Ledit_AltPreco->setText(preco);
}
