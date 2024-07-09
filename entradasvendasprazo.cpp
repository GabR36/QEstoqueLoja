#include "entradasvendasprazo.h"
#include "ui_entradasvendasprazo.h"

EntradasVendasPrazo::EntradasVendasPrazo(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EntradasVendasPrazo)
{
    ui->setupUi(this);
}

EntradasVendasPrazo::~EntradasVendasPrazo()
{
    delete ui;
}
