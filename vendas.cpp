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

    // colocar valores nos labels de lucro etc
    LabelLucro();
    //
    db.close();
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
    QString whereQuery = "";
    if (indexMes == 0 && indexDia == 0 && indexAno == 0){
        // todos os seletores estao em 'todos'
        modeloVendas2->setQuery("SELECT * FROM vendas2 ORDER BY id DESC");
        LabelLucro();
    }
    else{
        if (indexMes != 0 && indexDia != 0 && indexAno != 0){
            // nenhum seletor esta em 'todos'
            whereQuery = "WHERE strftime('%m', data_hora) = '" + valorMes + "' AND strftime('%d', data_hora) = '" + valorDia + "' AND strftime('%Y', data_hora) = '" + valorAno + "'";
            modeloVendas2->setQuery("SELECT * FROM vendas2 " + whereQuery);
            LabelLucro(whereQuery);
        }
        else{
            if (indexMes == 0 && indexDia != 0 && indexAno != 0){
                // so o mes esta com todos
                whereQuery = "WHERE strftime('%d', data_hora) = '" + valorDia + "' AND strftime('%Y', data_hora) = '" + valorAno + "'";
                modeloVendas2->setQuery("SELECT * FROM vendas2 " + whereQuery);
                LabelLucro(whereQuery);
            }
            else {
                if (indexMes == 0 && indexDia == 0 && indexAno != 0){
                    // mes e dia estao com todos
                    whereQuery = "WHERE strftime('%Y', data_hora) = '" + valorAno + "'";
                    modeloVendas2->setQuery("SELECT * FROM vendas2 " + whereQuery);
                    LabelLucro(whereQuery);
                }
                else{
                    if (indexMes == 0 && indexDia != 0 && indexAno == 0){
                        // mes e ano estao com todos
                        whereQuery = "WHERE strftime('%d', data_hora) = '" + valorDia + "'";
                        modeloVendas2->setQuery("SELECT * FROM vendas2 " + whereQuery);
                        LabelLucro(whereQuery);
                    }
                    else{
                        if (indexMes != 0 && indexDia == 0 && indexAno != 0){
                            // apenas dia esta com todos
                            whereQuery = "WHERE strftime('%m', data_hora) = '" + valorMes + "' AND strftime('%Y', data_hora) = '" + valorAno + "'";
                            modeloVendas2->setQuery("SELECT * FROM vendas2 " + whereQuery);
                            LabelLucro(whereQuery);
                        }
                        else{
                            if (indexMes != 0 && indexDia == 0 && indexAno == 0){
                                // dia e ano esta com todos
                                whereQuery = "WHERE strftime('%m', data_hora) = '" + valorMes + "'";
                                modeloVendas2->setQuery("SELECT * FROM vendas2 " + whereQuery);
                                LabelLucro(whereQuery);
                            }
                            else {
                                if (indexMes != 0 && indexDia != 0 && indexAno == 0){
                                    // apenas ano esta com todos
                                    whereQuery = "WHERE strftime('%m', data_hora) = '" + valorMes + "' AND strftime('%d', data_hora) = '" + valorDia + "'";
                                    modeloVendas2->setQuery("SELECT * FROM vendas2 " + whereQuery);
                                    LabelLucro(whereQuery);
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
    ui->Lbl_Total->setText(QString::number(total));
    ui->Lbl_Lucro->setText(QString::number(lucro));
    ui->Lbl_Quantidade->setText(QString::number(quantidadeVendas));
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
