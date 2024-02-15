#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>
#include "alterarproduto.h"
#include "QItemSelectionModel"
#include <qsqltablemodel.h>
#include "vendas.h"
#include <QDoubleValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // criar banco de dados e tabela se não foi ainda.
    db.setDatabaseName("estoque.db");
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados.";
    }
    QSqlQuery query;
    query.exec("CREATE TABLE produtos (id INTEGER PRIMARY KEY AUTOINCREMENT, quantidade INTEGER, descricao TEXT, preco DECIMAL(10,2))");
    if (query.isActive()) {
        qDebug() << "Tabela criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela: ";
    }
    query.exec("CREATE TABLE vendas2 (id INTEGER PRIMARY KEY AUTOINCREMENT, cliente TEXT, data_hora DATETIME DEFAULT CURRENT_TIMESTAMP, total DECIMAL(10,2))");
    if (query.isActive()) {
        qDebug() << "Tabela de vendas2 criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela de vendas2: ";
    }
    query.exec("CREATE TABLE produtos_vendidos (id INTEGER PRIMARY KEY AUTOINCREMENT, id_produto INTEGER, id_venda INTEGER, quantidade INTEGER, preco_vendido DECIMAL(10,2), FOREIGN KEY (id_produto) REFERENCES produtos(id), FOREIGN KEY (id_venda) REFERENCES vendas2(id))");
    if (query.isActive()) {
        qDebug() << "Tabela de produtos_vendidos criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela de produtos_vendidos: ";
    }
    qDebug() << db.tables();





    // mostrar na tabela da aplicaçao a tabela do banco de dados.
    atualizarTableview();
    QSqlDatabase::database().close();
    //
    ui->Ledit_Desc->setFocus();
    // Selecionar a primeira linha da tabela
    QModelIndex firstIndex = model->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);

    // ajustar tamanho colunas
    // coluna descricao
    ui->Tview_Produtos->setColumnWidth(2, 200);
    // coluna quantidade
    ui->Tview_Produtos->setColumnWidth(1, 85);



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::atualizarTableview(){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. atualizarTableView";
    }
    model->setQuery("SELECT * FROM produtos");
    ui->Tview_Produtos->setModel(model);
    db.close();
}

void MainWindow::on_Btn_Enviar_clicked()
{
    QString quantidadeProduto, descProduto, precoProduto;
    quantidadeProduto = ui->Ledit_Quantidade->text();
    descProduto = ui->Ledit_Desc->text();
    precoProduto = ui->Ledit_Preco->text();

    // Substitua ',' por '.' se necessário
    precoProduto.replace(',', '.');

    // Converta o texto para um número
    bool conversionOk;
    bool conversionOkQuant;
    double price = precoProduto.toDouble(&conversionOk);
    int quantidadeInt = quantidadeProduto.toInt(&conversionOkQuant);

    // Verifique se a conversão foi bem-sucedida e se o preço é maior que zero
    if (conversionOk && price >= 0)
    {
        if (conversionOkQuant){
            // adicionar ao banco de dados
            if(!db.open()){
                qDebug() << "erro ao abrir banco de dados. botao enviar.";
            }
            QSqlQuery query;

            query.prepare("INSERT INTO produtos (quantidade, descricao, preco) VALUES (:valor1, :valor2, :valor3)");
            query.bindValue(":valor1", quantidadeProduto);
            query.bindValue(":valor2", descProduto);
            query.bindValue(":valor3", precoProduto);
            if (query.exec()) {
                qDebug() << "Inserção bem-sucedida!";
            } else {
                qDebug() << "Erro na inserção: ";
            }
            atualizarTableview();
            QSqlDatabase::database().close();

            // limpar campos para nova inserçao
            ui->Ledit_Desc->clear();
            ui->Ledit_Quantidade->clear();
            ui->Ledit_Preco->clear();
            ui->Ledit_Desc->setFocus();
        }
        else{
            QMessageBox::warning(this, "Erro", "Por favor, insira uma quantidade válida.");
            ui->Ledit_Quantidade->clear();
        }
    }
    else
    {
        // Exiba uma mensagem de erro se o preço não for válido
        QMessageBox::warning(this, "Erro", "Por favor, insira um preço válido.");
        ui->Ledit_Preco->clear();
    }


}



void MainWindow::on_Btn_Delete_clicked()
{
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QString productId = idVariant.toString();
    QString productDesc = descVariant.toString();

    // Cria uma mensagem de confirmação
    QMessageBox::StandardButton resposta;
    resposta = QMessageBox::question(
        nullptr,
        "Confirmação",
        "Tem certeza que deseja excluir o produto:\n\n"
        "id: " + productId + "\n"
        "Descrição: " + productDesc,
        QMessageBox::Yes | QMessageBox::No
    );
    // Verifica a resposta do usuário
    if (resposta == QMessageBox::Yes) {

        // remover registro do banco de dados
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. botao deletar.";
        }
        QSqlQuery query;

        query.prepare("DELETE FROM produtos WHERE id = :valor1");
        query.bindValue(":valor1", productId);
        if (query.exec()) {
            qDebug() << "Delete bem-sucedido!";
        } else {
            qDebug() << "Erro no Delete: ";
        }
        atualizarTableview();
        QSqlDatabase::database().close();
    }
    else {
        // O usuário escolheu não deletar o produto
        qDebug() << "A exclusão do produto foi cancelada.";
    }
}


void MainWindow::on_Btn_Pesquisa_clicked()
{
    QString pesquisa = ui->Ledit_Pesquisa->text();
    // mostrar na tableview a consulta
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao pesquisar.";
    }
    model->setQuery("SELECT * FROM produtos WHERE descricao LIKE '%" + pesquisa + "%'");
    ui->Tview_Produtos->setModel(model);
    QSqlDatabase::database().close();
}




void MainWindow::on_Btn_Alterar_clicked()
{
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QVariant quantVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 1));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));
    QString productId = idVariant.toString();
    QString productQuant = quantVariant.toString();
    QString productDesc = descVariant.toString();
    QString productPreco = precoVariant.toString();
    qDebug() << productId;
    qDebug() << productPreco;
    // criar janela
    AlterarProduto *alterar = new AlterarProduto;
    alterar->janelaPrincipal = this;
    alterar->idAlt = productId;
    alterar->TrazerInfo(productDesc, productQuant, productPreco);
    alterar->show();
}


void MainWindow::on_Btn_Venda_clicked()
{
    Vendas *vendas = new Vendas;
    vendas->janelaPrincipal = this;
    vendas->show();
}

