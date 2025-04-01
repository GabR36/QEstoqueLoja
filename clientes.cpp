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

    ui->Tview_Clientes->setColumnWidth(0,50);
    ui->Tview_Clientes->setColumnWidth(1,150);//nome
    ui->Tview_Clientes->setColumnWidth(2,150);
    ui->Tview_Clientes->setColumnWidth(5,150);

    QItemSelectionModel *selectionModel = ui->Tview_Clientes->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &Clientes::atualizarInfos);

}

Clientes::~Clientes()
{
    delete ui;
}

void Clientes::on_Btn_Alterar_clicked()
{
    qDebug() << "Teste";
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
            QString clienteId = idVariant.toString();

            if(clienteId.toInt() > 1){
                // query para deletar a partir do id
                db.open();
                QSqlQuery query;
                query.prepare("DELETE FROM clientes WHERE id = :valor1");
                query.bindValue(":valor1", clienteId);
                query.exec();

                // atualizar
                model->setQuery("SELECT * FROM clientes");
            }else{
                QMessageBox::warning(this,"Erro","Não é possivel deletar o cliente 1!");
                return;

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
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. atualizarTableView";
    }
    model->setQuery("SELECT * FROM clientes");

    db.close();
}
int Clientes::getQuantCompras(int idCliente){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. geQuantVendas";
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM vendas2 where id_cliente = :id");
    query.bindValue(":id", idCliente);
    int index = 0;

    query.exec();
    while(query.next()){
        index += 1;
    }
    return index;

}
QString Clientes::getDataUltimoPagamento(int idCliente){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. getUltimoPagamento";
    }
    QSqlQuery query;

    query.prepare("SELECT ev.data_hora "
        "FROM entradas_vendas ev "
        "JOIN vendas2 v ON ev.id_venda = v.id "
        "WHERE v.id_cliente = :id "
        "ORDER BY ev.data_hora DESC "
        "LIMIT 1;");
    query.bindValue(":id", idCliente);

    if(!query.exec()) {
        qDebug() << "Erro na query:" << query.lastError().text();
        db.close();
        return QString();
    }

    QDateTime data;
    if(query.next()) {
        data = query.value(0).toDateTime();
    } else {
        db.close();
        return QString("Nenhum pagamento encontrado");
    }
    db.close();

    QString resultado = portugues.toString(data, "dddd, dd/MM/yyyy");

    return resultado;


}

double Clientes::getValorUltimoPagamento(int idCliente){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. getValorUltimoPagamento";
    }
    QSqlQuery query;

        query.prepare("SELECT ev.valor_final "
            "FROM entradas_vendas ev "
            "JOIN vendas2 v ON ev.id_venda = v.id "
            "WHERE v.id_cliente = :id "
            "ORDER BY ev.data_hora DESC "
            "LIMIT 1;");
    query.bindValue(":id", idCliente);

    if(!query.exec()) {
        qDebug() << "Erro na query:" << query.lastError().text();
        db.close();
        return 0;
    }
    double valor;
    if(query.next()) {
        valor = query.value(0).toDouble();
    } else {
        db.close();
        return 0;
    }
    db.close();

    return valor;
}
double Clientes::getValorDevido(int idCliente) {
    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados";
        return 0.0;
    }

    // 1. Soma o valor_final de todas as vendas do cliente
    QSqlQuery queryVendas;
    queryVendas.prepare(
        "SELECT SUM(valor_final) FROM vendas2 "
        "WHERE id_cliente = :id_cliente AND "
        "forma_pagamento = 'Prazo'"
        );
    queryVendas.bindValue(":id_cliente", idCliente);

    double totalVendas = 0.0;
    if (queryVendas.exec() && queryVendas.next()) {
        totalVendas = queryVendas.value(0).toDouble();
    } else {
        qDebug() << "Erro ao calcular total de vendas:" << queryVendas.lastError().text();
        db.close();
        return 0.0;
    }

    // 2. Soma o valor de todas as entradas relacionadas às vendas do cliente
    QSqlQuery queryEntradas;
    queryEntradas.prepare(
        "SELECT SUM(ev.valor_final) FROM entradas_vendas ev "
        "JOIN vendas2 v ON ev.id_venda = v.id "
        "WHERE v.id_cliente = :id_cliente"
        );
    queryEntradas.bindValue(":id_cliente", idCliente);

    double totalEntradas = 0.0;
    if (queryEntradas.exec() && queryEntradas.next()) {
        totalEntradas = queryEntradas.value(0).toDouble();
    } else {
        qDebug() << "Erro ao calcular total de entradas:" << queryEntradas.lastError().text();
        db.close();
        return 0.0;
    }

    db.close();

    // 3. Calcula o valor devido
    double valorDevido = totalVendas - totalEntradas;
    qDebug() << "Valor devido para cliente" << idCliente << ":" << valorDevido;

    return valorDevido;
}

void Clientes::on_Btn_abrirCompras_clicked()
{
    if(IDCLIENTE > 0){
        Vendas *janelaVendas = new Vendas(nullptr, IDCLIENTE);
        // MainWindow *mainwindow = new MainWindow;
        // janelaVendas->janelaPrincipal = mainwindow;
        janelaVendas->setWindowModality(Qt::ApplicationModal);
        janelaVendas->show();
    }else{
        QMessageBox::warning(this,"Erro","Selecione um Cliente na tabela para ver suas compras!");

    }
}

void Clientes::atualizarInfos(const QItemSelection &selected, const QItemSelection &)
{
    if (!selected.indexes().isEmpty()) {
        QModelIndex index = selected.indexes().first();  // Pega a primeira célula selecionada
        int idCliente = model->data(model->index(index.row(), 0)).toInt(); // Coluna 0 = id
        IDCLIENTE = idCliente;
        ui->Lbl_QuantCompras->setText(QString::number(getQuantCompras(idCliente)));
        ui->Lbl_DataUltimoPag->setText(getDataUltimoPagamento(idCliente));
        ui->Lbl_ValorUltimoPag->setText("R$ " + portugues.toString(getValorUltimoPagamento(idCliente)));
        ui->Lbl_valorTotalDevido->setText("R$ " + portugues.toString(getValorDevido(idCliente)));

    }
}
