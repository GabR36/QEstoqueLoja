#include "vendas.h"
#include "ui_vendas.h"
#include <QSqlQueryModel>

Vendas::Vendas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Vendas)
{
    ui->setupUi(this);
    QSqlQueryModel *modelo = new QSqlQueryModel;
    modelo->setQuery("SELECT * FROM vendas2");
    ui->Tview_Vendas2->setModel(modelo);
    modelo->setQuery("SELECT * FROM produtos_vendidos");
    ui->Tview_ProdutosVendidos->setModel(modelo);
}

Vendas::~Vendas()
{
    delete ui;
}
