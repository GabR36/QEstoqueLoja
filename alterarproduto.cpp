#include "alterarproduto.h"
#include "ui_alterarproduto.h"

AlterarProduto::AlterarProduto(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlterarProduto)
{
    ui->setupUi(this);
}

AlterarProduto::~AlterarProduto()
{
    delete ui;
}
