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
    QItemSelectionModel *selectionModel = ui->Tview_ProdutosSelecionados->selectionModel();
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

    // colocar a data atual no dateEdit
    ui->DateEdt_Venda->setDateTime(QDateTime::currentDateTime());
}

venda::~venda()
{
    delete ui;
}

void venda::on_Btn_SelecionarProduto_clicked()
{
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant quantVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 1));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));
    QString idProduto = idVariant.toString();
    QString descProduto = descVariant.toString();
    QString precoProduto = precoVariant.toString();
    QString quantProduto = quantVariant.toString();
    QVector<QString> registro1 = {idProduto, quantProduto, precoProduto};
    vetorIds.append(registro1);
    ui->Ledit_QuantVendido->clear();
    ui->Ledit_Preco->clear();
    qDebug() << vetorIds;
    // mostrar na tabela Selecionados
    modeloSelecionados.appendRow({new QStandardItem(idProduto), new QStandardItem(quantProduto), new QStandardItem(descProduto), new QStandardItem(precoProduto)});
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
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

    query.prepare("INSERT INTO vendas2 (cliente, total, data_hora) VALUES (:valor1, :valor2, :valor3)");
    query.bindValue(":valor1", cliente);
    query.bindValue(":valor2", totalSelecionados);
    // inserir a data do dateedit
    query.bindValue(":valor3", ui->DateEdt_Venda->dateTime().toString("dd-MM-yyyy HH:mm:ss"));
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
    QVariant idVariant = ui->Tview_ProdutosSelecionados->model()->data(ui->Tview_ProdutosSelecionados->model()->index(selectedIndex.row(), 0));
    QVariant quantVariant = ui->Tview_ProdutosSelecionados->model()->data(ui->Tview_ProdutosSelecionados->model()->index(selectedIndex.row(), 1));
    QVariant precoVariant = ui->Tview_ProdutosSelecionados->model()->data(ui->Tview_ProdutosSelecionados->model()->index(selectedIndex.row(), 3));
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


void venda::on_Ledit_QuantVendido_textChanged(const QString &arg1)
{
    // slot sempre que a quantidade for alterada, mudar o produto selecionado

    // pegar o produto selecionado
    QItemSelectionModel *selectionModel = ui->Tview_ProdutosSelecionados->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    int registroSelecionado = selectedIndex.row();
    // pegar o valor no line edit
    QString quantidade = ui->Ledit_QuantVendido->text();
    //
    vetorIds[registroSelecionado][1] = quantidade;
    qDebug() << vetorIds;
    QModelIndex quantidadeIndice = modeloSelecionados.index(registroSelecionado, 1);
    modeloSelecionados.setData(quantidadeIndice, quantidade);
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
}


void venda::on_Ledit_Preco_textChanged(const QString &arg1)
{
    // slot sempre que o preço for alterado, mudar o produto selecionado

    // pegar o produto selecionado
    QItemSelectionModel *selectionModel = ui->Tview_ProdutosSelecionados->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    int registroSelecionado = selectedIndex.row();
    // pegar o valor no line edit
    QString preco = ui->Ledit_Preco->text();
    //
    vetorIds[registroSelecionado][2] = preco;
    qDebug() << vetorIds;
    QModelIndex precoIndice = modeloSelecionados.index(registroSelecionado, 3);
    modeloSelecionados.setData(precoIndice, preco);
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
}

