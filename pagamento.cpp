#include "pagamento.h"
#include "ui_pagamento.h"

pagamento::pagamento(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::pagamento)
{
    ui->setupUi(this);
}

pagamento::~pagamento()
{
    delete ui;
}
