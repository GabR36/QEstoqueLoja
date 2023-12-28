#include "vendas.h"
#include "ui_vendas.h"
#include <QSqlQueryModel>
#include "venda.h"
#include <QDate>
#include <QtSql>

Vendas::Vendas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Vendas)
{
    ui->setupUi(this);
    atualizarTabelas();
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

    // adicionar items combobox com base nas datas disponiveis
    if (!db.open()) {
        qDebug() << "Erro ao abrir o banco de dados:";
    }
    QSqlQuery query;
    if (query.exec("SELECT DISTINCT strftime('%d', data_hora) AS data_disponivel FROM vendas2")) {
        // Iterar sobre os resultados
        while (query.next()) {
            QString dia = query.value(0).toString();
            qDebug() << dia;
        }
    } else {
        qDebug() << "Erro ao executar a consulta:" << query.lastError().text();
    }
    // adicionar meses ao seletor de meses combobox
    ui->CBox_Mes->addItem("Todos");
    ui->CBox_Mes->addItem("Janeiro");
    ui->CBox_Mes->addItem("Fevereiro");
    ui->CBox_Mes->addItem("Março");
    ui->CBox_Mes->addItem("Abril");
    ui->CBox_Mes->addItem("Maio");
    ui->CBox_Mes->addItem("Junho");
    ui->CBox_Mes->addItem("Julho");
    ui->CBox_Mes->addItem("Agosto");
    ui->CBox_Mes->addItem("Setembro");
    ui->CBox_Mes->addItem("Outubro");
    ui->CBox_Mes->addItem("Novembro");
    ui->CBox_Mes->addItem("Dezembro");

    // adicionar dias ao seletor de dias combobox
    ui->CBox_Dia->addItem("Todos");
    for (int i = 1; i < 32; i++){
        // ate 31 dias
        ui->CBox_Dia->addItem(QString::number(i));
    }


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
    inserirVenda->show();
}

void Vendas::atualizarTabelas(){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao venda.";
    }
    modeloVendas2->setQuery("SELECT * FROM vendas2");
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

void Vendas::on_CBox_Mes_activated(int index)
{
    qDebug() << index;

    int indexDia = ui->CBox_Dia->currentIndex();
    qDebug() << indexDia;

    if (index != 0 && indexDia != 0){
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. cbox_activated";
        }
        modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%m', data_hora) = '" + QString::number(index) + "' AND strftime('%d', data_hora) = '" + QString::number(indexDia) + "'");
        db.close();
    }
    else{
        if (index == 0 && indexDia == 0){
            // os dois estao selecionados 'todos'
            if(!db.open()){
                qDebug() << "erro ao abrir banco de dados. cbox_activated";
            }
            modeloVendas2->setQuery("SELECT * FROM vendas2");
            db.close();

        }
        else {
            if (indexDia == 0){
                // apenas o dia é todos
                if(!db.open()){
                    qDebug() << "erro ao abrir banco de dados. cbox_activated";
                }
                modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%m', data_hora) = '" + QString::number(index) + "'");
                db.close();
            }
            else{
                // apenas o mes é todos
                if(!db.open()){
                    qDebug() << "erro ao abrir banco de dados. cbox_activated (index todos)";
                }
                modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%d', data_hora) = '" + QString::number(indexDia) + "'");
                db.close();
            }
        }

    }
}


void Vendas::on_CBox_Dia_activated(int index)
{
    qDebug() << index;

    int indexMes = ui->CBox_Mes->currentIndex();
    qDebug() << indexMes;

    if (index != 0 && indexMes != 0){
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. cbox_activated";
        }
        modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%m', data_hora) = '" + QString::number(indexMes) + "' AND strftime('%d', data_hora) = '" + QString::number(index) + "'");
        db.close();
    }
    else{
        if (index == 0 && indexMes == 0){
            // os dois estao selecionados 'todos'
            if(!db.open()){
                qDebug() << "erro ao abrir banco de dados. cbox_activated";
            }
            modeloVendas2->setQuery("SELECT * FROM vendas2");
            db.close();

        }
        else {
            if (index == 0){
                // apenas o dia é todos
                if(!db.open()){
                    qDebug() << "erro ao abrir banco de dados. cbox_activated";
                }
                modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%m', data_hora) = '" + QString::number(indexMes) + "'");
                db.close();
            }
            else{
                // apenas o mes é todos
                if(!db.open()){
                    qDebug() << "erro ao abrir banco de dados. cbox_activated (index todos)";
                }
                modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%d', data_hora) = '" + QString::number(index) + "'");
                db.close();
            }
        }

    }
}

