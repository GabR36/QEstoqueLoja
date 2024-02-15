#include "venda.h"
#include "ui_venda.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QVector>
#include <QMessageBox>


venda::venda(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::venda)
{
    ui->setupUi(this);
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. construtor venda.";
    }
    modeloProdutos->setQuery("SELECT * FROM produtos");
    ui->Tview_Produtos->setModel(modeloProdutos);
    db.close();
    modeloSelecionados.setHorizontalHeaderItem(0, new QStandardItem("ID_Produto"));
    modeloSelecionados.setHorizontalHeaderItem(1, new QStandardItem("Quantidade_Vendida"));
    modeloSelecionados.setHorizontalHeaderItem(2, new QStandardItem("Descricao"));
    modeloSelecionados.setHorizontalHeaderItem(3, new QStandardItem("Preço Vendido"));
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
    // Selecionar a primeira linha da tabela
    QModelIndex firstIndex = modeloProdutos->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
    // Obter o modelo de seleção da tabela
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    // Conectar o sinal de seleção ao slot personalizado
    connect(selectionModel, &QItemSelectionModel::selectionChanged,this, &venda::handleSelectionChange);
    // ajustar tamanho colunas
    // coluna descricao
    ui->Tview_Produtos->setColumnWidth(2, 200);
    // coluna quantidade
    ui->Tview_Produtos->setColumnWidth(1, 85);
    // coluna quantidade vendida
    ui->Tview_ProdutosSelecionados->setColumnWidth(1, 180);
    // coluna descricao
    ui->Tview_ProdutosSelecionados->setColumnWidth(2, 250);
}

venda::~venda()
{
    delete ui;
}

void venda::on_Btn_SelecionarProduto_clicked()
{
    // pegar id do produto selecionado e quant do Ledit
    QString quantVendido = ui->Ledit_QuantVendido->text();
    QString precoVendido = ui->Ledit_Preco->text();
    // Substitua ',' por '.' se necessário
    precoVendido.replace(',', '.');

    // Converta o texto para um número
    bool conversionOk;
    bool conversionOkQuant;
    double price = precoVendido.toDouble(&conversionOk);
    double quantInt = quantVendido.toInt(&conversionOkQuant);

    // Verifique se a conversão foi bem-sucedida e se o preço é maior que zero
    if (conversionOk && price >= 0)
    {
        if(conversionOkQuant){
            QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
            QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
            QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
            QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
            QString idProduto = idVariant.toString();
            QString descProduto = descVariant.toString();
            QVector<QString> registro1 = {idProduto, quantVendido, precoVendido};
            vetorIds.append(registro1);
            ui->Ledit_QuantVendido->clear();
            ui->Ledit_Preco->clear();
            qDebug() << vetorIds;
            // mostrar na tabela Selecionados
            modeloSelecionados.appendRow({new QStandardItem(idProduto), new QStandardItem(quantVendido), new QStandardItem(descProduto), new QStandardItem(precoVendido)});
            ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
        }
        else {
            QMessageBox::warning(this, "Erro", "Por favor, insira uma quantidade válida.");
        }

    }
    else
    {
        // Exiba uma mensagem de erro se o preço não for válido
        QMessageBox::warning(this, "Erro", "Por favor, insira um preço válido.");
    }
}


void venda::on_BtnBox_Venda_accepted()
{
    QString cliente = ui->Ledit_Cliente->text();
    float totalSelecionados = 0;
    for (const QVector<QString> &registro : vetorIds) {
        totalSelecionados = totalSelecionados + registro[1].toFloat() * registro[2].toFloat();
        qDebug() << registro[2].toFloat();
    }
    // adicionar ao banco de dados
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao aceitar venda.";
        return;
    }
    QSqlQuery query;

    query.prepare("INSERT INTO vendas2 (cliente, total) VALUES (:valor1, :valor2)");
    query.bindValue(":valor1", cliente);
    query.bindValue(":valor2", totalSelecionados);
    QString idVenda;
    if (query.exec()) {
        idVenda = query.lastInsertId().toString();
        qDebug() << "Inserção bem-sucedida!";
    } else {
        qDebug() << "Erro na inserção: ";
    }
    // adicionar ao banco de dados
    for (const QVector<QString> &registro : vetorIds) {
        query.prepare("INSERT INTO produtos_vendidos (id_produto, quantidade, preco_vendido, id_venda) VALUES (:valor1, :valor2, :valor3, :valor4)");
        query.bindValue(":valor1", registro[0]);
        query.bindValue(":valor2", registro[1]);
        query.bindValue(":valor3", registro[2]);
        query.bindValue(":valor4", idVenda);
        if (query.exec()) {
            qDebug() << "Inserção prod_vendidos bem-sucedida!";
        } else {
            qDebug() << "Erro na inserção prod_vendidos: ";
        }
        query.prepare("UPDATE produtos SET quantidade = quantidade - :valor2 WHERE id = :valor1");
        query.bindValue(":valor1", registro[0]);
        query.bindValue(":valor2", registro[1]);
        if (query.exec()) {
            qDebug() << "update quantidade bem-sucedida!";
        } else {
            qDebug() << "Erro na update quantidade: ";
        }
    }
    db.close();
    janelaVenda->atualizarTabelas();
    janelaPrincipal->atualizarTableview();
}

void venda::handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected) {
    // Este slot é chamado sempre que a seleção na tabela muda
    Q_UNUSED(deselected);

    qDebug() << "Registro(s) selecionado(s):";

    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. handleselectionchange Venda";
    }
    QModelIndex selectedIndex = selected.indexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QVariant quantVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 1));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));
    QString productId = idVariant.toString();
    QString productQuant = quantVariant.toString();
    QString productPreco = precoVariant.toString();
    ui->Ledit_QuantVendido->setText(productQuant);
    ui->Ledit_Preco->setText(productPreco);
    qDebug() << productId;
    db.close();
}


void venda::on_Btn_Pesquisa_clicked()
{
    QString pesquisa = ui->Ledit_Pesquisa->text();
    // mostrar na tableview a consulta
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao pesquisar.";
    }
    modeloProdutos->setQuery("SELECT * FROM produtos WHERE descricao LIKE '%" + pesquisa + "%'");
    ui->Tview_Produtos->setModel(modeloProdutos);
    QSqlDatabase::database().close();
}

