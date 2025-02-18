#include "clientes.h"
#include "ui_clientes.h"
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QMessageBox>
#include "alterarcliente.h"

Clientes::Clientes(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Clientes)
{
    ui->setupUi(this);

    model->setQuery("SELECT * FROM clientes");
    ui->Tview_Clientes->setModel(model);
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Nome"));
    model->setHeaderData(2, Qt::Horizontal, tr("Email"));
    model->setHeaderData(3, Qt::Horizontal, tr("Telefone"));
    model->setHeaderData(4, Qt::Horizontal, tr("Endereço"));
    model->setHeaderData(5, Qt::Horizontal, tr("CPF"));
    model->setHeaderData(6, Qt::Horizontal, tr("Data Nascimento"));
    model->setHeaderData(7, Qt::Horizontal, tr("Data Cadastro"));
}

Clientes::~Clientes()
{
    delete ui;
}

void Clientes::on_Btn_Alterar_clicked()
{
    qDebug() << "Teste";
    if(ui->Tview_Clientes->selectionModel()->isSelected(ui->Tview_Clientes->currentIndex())){
        // obter id, nome, etc. selecionado
        QItemSelectionModel *selectionModel = ui->Tview_Clientes->selectionModel();
        QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
        QVariant idVariant = ui->Tview_Clientes->model()->data(ui->Tview_Clientes->model()->index(selectedIndex.row(), 0));
        QString clienteId = idVariant.toString();

        AlterarCliente *alterarJanela = new AlterarCliente(nullptr, clienteId);
        alterarJanela->show();
    }
    else{
        QMessageBox::warning(this,"Erro","Selecione um produto antes de alterar!");
    }
}

