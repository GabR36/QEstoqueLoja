#include "clientes.h"
#include "ui_clientes.h"

Clientes::Clientes(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Clientes)
{
    ui->setupUi(this);
}

Clientes::~Clientes()
{
    delete ui;
}
