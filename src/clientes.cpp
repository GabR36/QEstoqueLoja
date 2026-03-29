#include "clientes.h"
#include "ui_clientes.h"
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QMessageBox>
#include "alterarcliente.h"
#include <QSqlQuery>
#include "inserircliente.h"
#include <QDateTime>
#include <QSqlError>
#include "vendas.h"
#include "mainwindow.h"
#include "delegatehora.h"

Clientes::Clientes(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Clientes)
{
    ui->setupUi(this);


    model = new QSqlQueryModel(this);
    cliServ.listarClientes(model);
    ui->Tview_Clientes->setModel(model);

    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Nome"));
    model->setHeaderData(2, Qt::Horizontal, tr("Email"));
    model->setHeaderData(3, Qt::Horizontal, tr("Telefone"));
    model->setHeaderData(4, Qt::Horizontal, tr("Endereço"));
    model->setHeaderData(5, Qt::Horizontal, tr("CPF"));
    model->setHeaderData(6, Qt::Horizontal, tr("Data Nascimento"));
    model->setHeaderData(7, Qt::Horizontal, tr("Data Cadastro"));

    ui->Tview_Clientes->setColumnWidth(0,50);
    ui->Tview_Clientes->setColumnWidth(1,150);//nome
    ui->Tview_Clientes->setColumnWidth(2,150);
    ui->Tview_Clientes->setColumnWidth(5,150);

    QItemSelectionModel *selectionModel = ui->Tview_Clientes->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &Clientes::atualizarInfos);

    DelegateHora *delegateData = new DelegateHora;
    ui->Tview_Clientes->setItemDelegateForColumn(7,delegateData);

}

Clientes::~Clientes()
{
    delete ui;
}

void Clientes::on_Btn_Alterar_clicked()
{
    if(ui->Tview_Clientes->selectionModel()->isSelected(ui->Tview_Clientes->currentIndex())){
        // obter id selecionado
        QItemSelectionModel *selectionModel = ui->Tview_Clientes->selectionModel();
        QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
        QVariant idVariant = ui->Tview_Clientes->model()->data(ui->Tview_Clientes->model()->index(selectedIndex.row(), 0));
        QString clienteId = idVariant.toString();
        if(clienteId.toInt() > 1){
            AlterarCliente *alterarJanela = new AlterarCliente(nullptr, clienteId);
            alterarJanela->setWindowModality(Qt::ApplicationModal);
            connect(alterarJanela, &AlterarCliente::clienteAtualizado, this, &Clientes::atualizarTableview);
            alterarJanela->show();
        }else{
            QMessageBox::warning(this,"Erro","Não é possivel alterar o cliente 1!");
            return;
        }

    }
    else{
        QMessageBox::warning(this,"Erro","Selecione um cliente antes de alterar!");
    }
}

void Clientes::atualizarTabelaClientes(){
    cliServ.listarClientes(model);
}



void Clientes::on_Btn_Deletar_clicked()
{
    if(ui->Tview_Clientes->selectionModel()->isSelected(ui->Tview_Clientes->currentIndex())){
        // Cria uma mensagem de confirmação
        QMessageBox::StandardButton resposta;
        resposta = QMessageBox::question(
            nullptr,
            "Confirmação",
            "Tem certeza que deseja deletar o cliente?",
            QMessageBox::Yes | QMessageBox::No
            );
        // Verifica a resposta do usuário
        if (resposta == QMessageBox::Yes) {

            // obter id selecionado
            QItemSelectionModel *selectionModel = ui->Tview_Clientes->selectionModel();
            QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
            QVariant idVariant = ui->Tview_Clientes->model()->data(ui->Tview_Clientes->model()->index(selectedIndex.row(), 0));
            qlonglong clienteId = idVariant.toLongLong();

            auto result = cliServ.deletarCliente(clienteId);
            if(!result.ok){
                QMessageBox::warning(this,"Erro",result.msg);
            }else{
                atualizarTabelaClientes();
            }
        }
    }
    else{
        QMessageBox::warning(this,"Erro","Selecione um cliente antes de deletar!");
    }
}


void Clientes::on_Btn_Novo_clicked()
{
    InserirCliente *inserircliente = new InserirCliente();
    inserircliente->setWindowModality(Qt::ApplicationModal);
    connect(inserircliente, &InserirCliente::clienteInserido, this, &Clientes::atualizarTableview);

    inserircliente->show();
}
void Clientes::atualizarTableview(){
    atualizarTabelaClientes();
}

void Clientes::atualizarInfosSinal(){


    qlonglong idCliente = IDCLIENTE;
    QDateTime dataUltimoPag = entradaServ.getDataUltimoPagamentoFromCliente(idCliente);
    QString dataFormatada = portugues.toString(dataUltimoPag, "dddd, dd/MM/yyyy");
    QString ValorTotalDevidoFormatado = portugues.toString(financeiroServ.getValorTotalDevidoFromCliente(idCliente));
    QString valorUltimoPagFormatado = portugues.toString(entradaServ.getValorUltimoPagamentoFromCliente(idCliente));
    QString quantCompras = QString::number(vendaServ.getQuantidadeComprasCliente(idCliente));

    ui->Lbl_QuantCompras->setText(quantCompras);
    ui->Lbl_DataUltimoPag->setText(dataFormatada);
    ui->Lbl_ValorUltimoPag->setText("R$ " + valorUltimoPagFormatado);
    ui->Lbl_valorTotalDevido->setText("R$ " + ValorTotalDevidoFormatado);
}
void Clientes::on_Btn_abrirCompras_clicked()
{
    if(IDCLIENTE > 0){
        Vendas *janelaVendas = new Vendas(nullptr, IDCLIENTE);
        // MainWindow *mainwindow = new MainWindow;
        // janelaVendas->janelaPrincipal = mainwindow;
        janelaVendas->setWindowModality(Qt::ApplicationModal);
        janelaVendas->show();
        connect(janelaVendas, &Vendas::pagamentosConcluidos,
                this, &Clientes::atualizarInfosSinal);
        connect(janelaVendas, &Vendas::devolvidoProduto,
                this, &Clientes::atualizarInfosSinal);
        connect(janelaVendas, &Vendas::vendaDeletada,
                this, &Clientes::atualizarInfosSinal);

    }else{
        QMessageBox::warning(this,"Erro","Selecione um Cliente na tabela para ver suas compras!");

    }
}


void Clientes::atualizarInfos(const QItemSelection &selected, const QItemSelection &)
{
    if (!selected.indexes().isEmpty()) {

        QModelIndex index = selected.indexes().first();  // Pega a primeira célula selecionada
        qlonglong idCliente = model->data(model->index(index.row(), 0)).toLongLong(); // Coluna 0 = id
        QDateTime dataUltimoPag = entradaServ.getDataUltimoPagamentoFromCliente(idCliente);
        QString dataFormatada = portugues.toString(dataUltimoPag, "dddd, dd/MM/yyyy");
        QString ValorTotalDevidoFormatado = portugues.toString(financeiroServ.getValorTotalDevidoFromCliente(idCliente));
        QString valorUltimoPagFormatado = portugues.toString(entradaServ.getValorUltimoPagamentoFromCliente(idCliente));
        QString quantCompras = QString::number(vendaServ.getQuantidadeComprasCliente(idCliente));
        IDCLIENTE = idCliente;
        ui->Lbl_QuantCompras->setText(quantCompras);
        ui->Lbl_DataUltimoPag->setText(dataFormatada);
        ui->Lbl_ValorUltimoPag->setText("R$ " + valorUltimoPagFormatado);
        ui->Lbl_valorTotalDevido->setText("R$ " + ValorTotalDevidoFormatado);
    }
}

void Clientes::on_Ledit_Pesquisa_textChanged(const QString &arg1)
{
    QString inputText = ui->Ledit_Pesquisa->text();
    cliServ.pesquisar(model, inputText);
}

