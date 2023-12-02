#include "vendas.h"
#include "ui_vendas.h"
#include <QSqlQueryModel>
#include "venda.h"

Vendas::Vendas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Vendas)
{
    ui->setupUi(this);
    db2.setDatabaseName("estoque.db");
    atualizarTabelas();
}

Vendas::~Vendas()
{
    delete ui;
}

void Vendas::on_Btn_InserirVenda_clicked()
{
    venda *inserirVenda = new venda;
    inserirVenda->janelaVenda = this;
    inserirVenda->db = db;
    inserirVenda->show();
}

void Vendas::atualizarTabelas(){
    if(!db2.open()){
        qDebug() << "erro ao abrir banco de dados. botao venda.";
    }
    modeloVendas2->setQuery("SELECT * FROM vendas2");
    ui->Tview_Vendas2->setModel(modeloVendas2);
    modeloProdVendidos->setQuery("SELECT * FROM produtos_vendidos");
    ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
    QSqlDatabase::database().close();
}

