#include "venda.h"
#include "ui_venda.h"

venda::venda(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::venda)
{
    ui->setupUi(this);
}

venda::~venda()
{
    delete ui;
}

void venda::on_Btn_SelecionarProduto_clicked()
{

}

