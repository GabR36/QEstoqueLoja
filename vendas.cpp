#include "vendas.h"
#include "ui_vendas.h"
#include <QSqlQueryModel>

Vendas::Vendas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Vendas)
{
    ui->setupUi(this);
    QSqlQueryModel *modeloVendas2 = new QSqlQueryModel;
    modeloVendas2->setQuery("SELECT * FROM vendas2");
    ui->Tview_Vendas2->setModel(modeloVendas2);
    QSqlQueryModel *modeloProdVendidos = new QSqlQueryModel;
    modeloProdVendidos->setQuery("SELECT * FROM produtos_vendidos");
    ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
}

Vendas::~Vendas()
{
    delete ui;
}

void Vendas::on_Btn_InserirVenda_clicked()
{

}

