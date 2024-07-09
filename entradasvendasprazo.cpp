#include "entradasvendasprazo.h"
#include "ui_entradasvendasprazo.h"

EntradasVendasPrazo::EntradasVendasPrazo(QWidget *parent, QString id_venda)
    : QDialog(parent)
    , ui(new Ui::EntradasVendasPrazo)
{
    ui->setupUi(this);
    QString a = id_venda;
    ui->label->setText(a);
}

EntradasVendasPrazo::~EntradasVendasPrazo()
{
    delete ui;
}
