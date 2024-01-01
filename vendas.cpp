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
    QVector<QString> dias;
    QVector<int> meses;
    QVector<QString> anos;
    if (!db.open()) {
        qDebug() << "Erro ao abrir o banco de dados:";
    }
    QSqlQuery query;
    if (query.exec("SELECT DISTINCT strftime('%d', data_hora) FROM vendas2")) {
        // Iterar sobre os resultados
        while (query.next()) {
            QString dia = query.value(0).toString();
            dias.push_back(dia);
        }
    } else {
        qDebug() << "Erro ao executar a consulta:" << query.lastError().text();
    }
    if (query.exec("SELECT DISTINCT strftime('%m', data_hora) FROM vendas2")) {
        // Iterar sobre os resultados
        while (query.next()) {
            int mes = query.value(0).toInt();
            meses.push_back(mes);
        }
    } else {
        qDebug() << "Erro ao executar a consulta:" << query.lastError().text();
    }
    if (query.exec("SELECT DISTINCT strftime('%Y', data_hora) FROM vendas2")) {
        // Iterar sobre os resultados
        while (query.next()) {
            QString ano = query.value(0).toString();
            anos.push_back(ano);
        }
    } else {
        qDebug() << "Erro ao executar a consulta:" << query.lastError().text();
    }
    db.close();
    qDebug() << anos;
    qDebug() << dias;
    qDebug() << meses;

    // adicionar meses ao seletor de mes combobox
    // transformar os numeros de mes em nomes de meses
    mapaMeses = {
        {"Janeiro", 1},
        {"Fevereiro", 2},
        {"Março", 3},
        {"Abril", 4},
        {"Maio", 5},
        {"Junho", 6},
        {"Julho", 7},
        {"Agosto", 8},
        {"Setembro", 9},
        {"Outubro", 10},
        {"Novembro", 11},
        {"Dezembro", 12}
    };
    ui->CBox_Mes->addItem("Todos");
    for(int mes : meses){
            ui->CBox_Mes->addItem(mapaMeses.key(mes));
    }

    // adicionar dias ao seletor de dias combobox
    ui->CBox_Dia->addItem("Todos");
    for(QString &dia : dias){
        ui->CBox_Dia->addItem(dia);
    }

    // adicionar os anos ao seletor de anos combobox
    ui->CBox_Ano->addItem("Todos");
    for(QString &ano : anos){
        ui->CBox_Ano->addItem(ano);
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
    queryCBox(ui->CBox_Ano->currentIndex(), index, ui->CBox_Dia->currentIndex());
}


void Vendas::on_CBox_Dia_activated(int index)
{
    queryCBox(ui->CBox_Ano->currentIndex(), ui->CBox_Mes->currentIndex(), index);
}


void Vendas::on_CBox_Ano_activated(int index)
{
    queryCBox(index, ui->CBox_Mes->currentIndex(), ui->CBox_Dia->currentIndex());
}

void Vendas::queryCBox (int indexAno, int indexMes, int indexDia){
    QString valorAno = ui->CBox_Ano->itemText(indexAno);
    QString valorMes = QString::number(mapaMeses.value(ui->CBox_Mes->itemText(indexMes)));
    QString valorDia = ui->CBox_Dia->itemText(indexDia);

    qDebug() << "valores indexes ano, mes, dia";
    qDebug() << valorAno;
    qDebug() << valorMes;
    qDebug() << valorDia;

    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. cbox_activated";
    }

    if (indexMes == 0 && indexDia == 0 && indexAno == 0){
        // todos os seletores estao em 'todos'
        modeloVendas2->setQuery("SELECT * FROM vendas2");
    }
    else{
        if (indexMes != 0 && indexDia != 0 && indexAno != 0){
            // nenhum seletor esta em 'todos'
            modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%m', data_hora) = '" + valorMes + "' AND strftime('%d', data_hora) = '" + valorDia + "' AND strftime('%Y', data_hora) = '" + valorAno + "'");
        }
        else{
            if (indexMes == 0 && indexDia != 0 && indexAno != 0){
                // so o mes esta com todos
                modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%d', data_hora) = '" + valorDia + "' AND strftime('%Y', data_hora) = '" + valorAno + "'");
            }
            else {
                if (indexMes == 0 && indexDia == 0 && indexAno != 0){
                    // mes e dia estao com todos
                    modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%Y', data_hora) = '" + valorAno + "'");
                }
                else{
                    if (indexMes == 0 && indexDia != 0 && indexAno == 0){
                        // mes e ano estao com todos
                        modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%d', data_hora) = '" + valorDia + "'");
                    }
                    else{
                        if (indexMes != 0 && indexDia == 0 && indexAno != 0){
                            // apenas dia esta com todos
                            modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%m', data_hora) = '" + valorMes + "' AND strftime('%Y', data_hora) = '" + valorAno + "'");
                        }
                        else{
                            if (indexMes != 0 && indexDia == 0 && indexAno == 0){
                                // dia e ano esta com todos
                                modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%m', data_hora) = '" + valorMes + "'");
                            }
                            else {
                                if (indexMes != 0 && indexDia != 0 && indexAno == 0){
                                    // apenas ano esta com todos
                                    modeloVendas2->setQuery("SELECT * FROM vendas2 WHERE strftime('%m', data_hora) = '" + valorMes + "' AND strftime('%d', data_hora) = '" + valorDia + "'");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    db.close();

}

