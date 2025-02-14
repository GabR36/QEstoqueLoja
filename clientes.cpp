#include "clientes.h"
#include "ui_clientes.h"
#include <QSqlQueryModel>
#include <QSqlDatabase>

Clientes::Clientes(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Clientes)
{
    ui->setupUi(this);

    model->setQuery("SELECT * FROM clientes");
    ui->Tview_Clientes->setModel(model);
}

Clientes::~Clientes()
{
    delete ui;
}
