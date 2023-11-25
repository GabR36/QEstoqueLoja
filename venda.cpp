#include "venda.h"
#include "ui_venda.h"
#include <QSqlQueryModel>
#include <QSqlQuery>

venda::venda(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::venda)
{
    ui->setupUi(this);
    QSqlQueryModel *modeloProdutos = new QSqlQueryModel;
    modeloProdutos->setQuery("SELECT * FROM produtos");
    ui->Tview_Produtos->setModel(modeloProdutos);
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
    QString idProduto = idVariant.toString();
    vetorIds.push_back(idProduto);
    QString idQuery;
    for (int i = 0; i < vetorIds.size(); i++){
        idQuery = idQuery + " id = " + vetorIds[i] + " OR";
    }
    int ultimoEspaço = idQuery.lastIndexOf(' ');
    idQuery = idQuery.left(ultimoEspaço);
    QSqlQueryModel *modeloSelecionados = new QSqlQueryModel;
    modeloSelecionados->setQuery("SELECT * FROM produtos WHERE" + idQuery);
    ui->Tview_ProdutosSelecionados->setModel(modeloSelecionados);
    qDebug() << idQuery;
    qDebug() << vetorIds;
}


void venda::on_buttonBox_accepted()
{
    QString cliente = ui->Ledit_Cliente->text();
    // adicionar ao banco de dados
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao enviar.";
    }
    QSqlQuery query;

    query.prepare("INSERT INTO vendas2 (cliente) VALUES (:valor1)");
    query.bindValue(":valor1", cliente);

    if (query.exec()) {
        qDebug() << "Inserção bem-sucedida!";
    } else {
        qDebug() << "Erro na inserção: ";
    }

    query.prepare("INSERT INTO produtos_vendidos ");
    QSqlDatabase::database().close();
}

