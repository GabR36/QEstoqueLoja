#include "produtotableview.h"
#include <QHeaderView>
#include "../delegateprecof2.h"
#include "../customdelegate.h"
#include <QSqlError>
#include "../infojanelaprod.h"

ProdutoTableView::ProdutoTableView(QWidget *parent) : QTableView(parent)
{
   // qDebug() << "Construtor ProdutoTableView chamado"; // Debug 1
    model = new QSqlQueryModel(this);
    configurar();
}

ProdutoTableView::~ProdutoTableView()
{
    qDebug() << "Destrutor ProdutoTableView chamado"; // Debug 2
}

void ProdutoTableView::configurar()
{

    db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Banco de dados não está aberto!"; // Debug 4
        return;
    }

    model->setQuery("SELECT * FROM produtos ORDER BY id DESC");
    if (model->lastError().isValid()) {
        qDebug() << "Erro na query:" << model->lastError().text(); // Debug 5
        return;
    }

    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Quantidade"));
    model->setHeaderData(2, Qt::Horizontal, tr("Descrição"));
    model->setHeaderData(3, Qt::Horizontal, tr("Preço"));
    model->setHeaderData(4, Qt::Horizontal, tr("Código de Barras"));
    model->setHeaderData(5, Qt::Horizontal, tr("NF"));

    setModel(model);
    horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: rgb(33, 105, 149); }");
    DelegatePrecoF2 *delegatePreco = new DelegatePrecoF2(this);
    setItemDelegateForColumn(3, delegatePreco);

    CustomDelegate *delegateVermelho = new CustomDelegate(this);
    setItemDelegateForColumn(1, delegateVermelho);

    setColumnWidth(2, 200);

    verticalHeader()->setVisible(false);
    // Configurações de seleção
    setSelectionMode(QAbstractItemView::SingleSelection);  // Seleciona apenas um item por vez
    setSelectionBehavior(QAbstractItemView::SelectRows);  // Seleciona linhas inteiras

    connect(this, &QTableView::doubleClicked,
            this, &ProdutoTableView::verProd);

}
QSqlQueryModel *ProdutoTableView::getModel(){
    return model;

}
int ProdutoTableView::getIdProdSelected(){
    QItemSelectionModel *selectionModel = this->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex idIndex = model->index(selectedRow, 0);

        int id = model->data(idIndex).toInt();
        return id;
    }
}
void ProdutoTableView::verProd(){
    int id = getIdProdSelected();
    InfoJanelaProd *janelaProd = new InfoJanelaProd(this, id);
    janelaProd->show();
}



