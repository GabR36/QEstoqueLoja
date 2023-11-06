#include "vender.h"
#include "ui_vender.h"

Vender::Vender(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Vender)
{
    ui->setupUi(this);
}

Vender::~Vender()
{
    delete ui;
}
