#include "vendas.h"
#include "ui_vendas.h"
#include <QSqlQueryModel>
#include "venda.h"
#include <QDate>
#include <QtSql>
#include <QMessageBox>

Vendas::Vendas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Vendas)
{
    ui->setupUi(this);
    atualizarTabelas();
    ui->Tview_Vendas2->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    ui->Tview_ProdutosVendidos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");

    // Selecionar a primeira linha das tabelas
    QModelIndex firstIndex = modeloVendas2->index(0, 0);
    ui->Tview_Vendas2->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
    QModelIndex firstIndex2 = modeloProdVendidos->index(0, 0);
    ui->Tview_ProdutosVendidos->selectionModel()->select(firstIndex2, QItemSelectionModel::Select);
    // Obter o modelo de seleção da tabela
    QItemSelectionModel *selectionModel = ui->Tview_Vendas2->selectionModel();
    // Conectar o sinal de seleção ao slot personalizado
    connect(selectionModel, &QItemSelectionModel::selectionChanged,this, &Vendas::handleSelectionChange);
    // ajustar tamanho colunas
    // coluna data
    ui->Tview_Vendas2->setColumnWidth(2, 150);
    // coluna cliente
    ui->Tview_Vendas2->setColumnWidth(1, 100);
    // coluna descricao
    ui->Tview_ProdutosVendidos->setColumnWidth(0, 200);
    // coluna quantidade
    ui->Tview_ProdutosVendidos->setColumnWidth(1, 85);

    // conectar banco de dados para consultar as datas mais antigas e mais novas das vendas
    // para mostrar nos qdateedits
    if (!db.open()) {
        qDebug() << "Error: Could not connect to database.";
    }

    QSqlQuery query;

    QPair<QDate, QDate> dateRange;
    // data antiga
    query.exec("SELECT MIN(data_hora) FROM vendas2");
    if (query.next()) {
        dateRange.first = query.value(0).toDate();
    }

    // data nova
    query.exec("SELECT MAX(data_hora) FROM vendas2");
    if (query.next()) {
        dateRange.second = query.value(0).toDate();
    }

    qDebug() << dateRange;
}

Vendas::~Vendas()
{
    delete ui;
}

void Vendas::on_Btn_InserirVenda_clicked()
{
    venda *inserirVenda = new venda;
    inserirVenda->janelaVenda = this;
    inserirVenda->janelaPrincipal = janelaPrincipal;
    inserirVenda->setWindowModality(Qt::ApplicationModal);
    inserirVenda->show();
}

void Vendas::atualizarTabelas(){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao venda.";
    }
    modeloVendas2->setQuery("SELECT * FROM vendas2 ORDER BY id DESC");
    ui->Tview_Vendas2->setModel(modeloVendas2);
    modeloProdVendidos->setQuery("SELECT * FROM produtos_vendidos");
    ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
    db.close();
}


void Vendas::handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected) {
    // Este slot é chamado sempre que a seleção na tabela muda
    Q_UNUSED(deselected);

    qDebug() << "Registro(s) selecionado(s):";

    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. handleselectionchange";
    }
    QModelIndex selectedIndex = selected.indexes().first();
    QVariant idVariant = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 0));
    QString productId = idVariant.toString();
    modeloProdVendidos->setQuery("SELECT produtos.descricao, produtos_vendidos.quantidade, produtos_vendidos.preco_vendido FROM produtos_vendidos JOIN produtos ON produtos_vendidos.id_produto = produtos.id WHERE id_venda = " + productId);
    ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
    db.close();
}

void Vendas::LabelLucro(QString whereQuery){
    // colocar valores nos labels de lucro etc
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. labelLucro";
    }
    QSqlQuery query;
    float total = 0;
    int quantidadeVendas = 0;
    if (query.exec("SELECT SUM(total) FROM vendas2 " + whereQuery)) {
        while (query.next()) {
            total = query.value(0).toFloat();
        }
    }
    if (query.exec("SELECT COUNT(*) FROM vendas2 " + whereQuery)) {
        while (query.next()) {
            quantidadeVendas = query.value(0).toInt();
        }
    }
    float lucro = total*0.28; // assumindo que o lucro é 40% do preco de venda
    ui->Lbl_Total->setText(portugues.toString(total));
    ui->Lbl_Lucro->setText(portugues.toString(lucro));
    ui->Lbl_Quantidade->setText(portugues.toString(quantidadeVendas));
    db.close();
}

void Vendas::LabelLucro(){
    // para ser chamada sem argumentos
    LabelLucro(QString());
}


void Vendas::on_Btn_DeletarVenda_clicked()
{
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Vendas2->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 0));
    QVariant valorVariant = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 3));
    QString productId = idVariant.toString();
    QString productValor = valorVariant.toString();

    // Cria uma mensagem de confirmação
    QMessageBox::StandardButton resposta;
    resposta = QMessageBox::question(
        nullptr,
        "Confirmação",
        "Tem certeza que deseja excluir a venda:\n\n"
        "id: " + productId + "\n"
                          "Valor total: " + productValor,
        QMessageBox::Yes | QMessageBox::No
        );
    // Verifica a resposta do usuário
    if (resposta == QMessageBox::Yes) {

        // remover registro do banco de dados
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. botao deletar.";
        }
        QSqlQuery query;

        // pegar os ids dos produtos e quantidades para adicionar depois
        query.prepare("SELECT id_produto, quantidade FROM produtos_vendidos WHERE id_venda = :valor1");
        query.bindValue(":valor1", productId);
        if (query.exec()) {
            qDebug() << "query bem-sucedido!";
        } else {
            qDebug() << "Erro no query: ";
        }
        while (query.next()){
            QString idProduto = query.value(0).toString();
            QString quantProduto = query.value(1).toString();
            qDebug() << idProduto;
            qDebug() <<  quantProduto;
            QSqlQuery updatequery;
            updatequery.prepare("UPDATE produtos SET quantidade = quantidade + :quantidade WHERE id = :id");
            updatequery.bindValue(":id", idProduto);
            updatequery.bindValue(":quantidade", quantProduto);
            if (updatequery.exec()) {
                qDebug() << "query UPDATE bem-sucedido!";
            } else {
                qDebug() << "Erro no query UPDATE: ";
            }
        }


        // deletar a venda
        query.prepare("DELETE FROM vendas2 WHERE id = :valor1");
        query.bindValue(":valor1", productId);
        if (query.exec()) {
            qDebug() << "Delete bem-sucedido!";
        } else {
            qDebug() << "Erro no Delete: ";
        }
        // deletar os produtos vendidos
        query.prepare("DELETE FROM produtos_vendidos WHERE id_venda = :valor1");
        query.bindValue(":valor1", productId);
        if (query.exec()) {
            qDebug() << "Delete bem-sucedido!";
        } else {
            qDebug() << "Erro no Delete: ";
        }      
        atualizarTabelas();
        db.close();
    }
    else {
        // O usuário escolheu não deletar o produto
        qDebug() << "A exclusão da venda foi cancelada.";
    }
}
