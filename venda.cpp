#include "venda.h"
#include "ui_venda.h"
#include "customdelegate.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QVector>
#include <QMessageBox>
#include "pagamentovenda.h"
#include <QDoubleValidator>
#include <QMenu>



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
    modeloProdutos->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modeloProdutos->setHeaderData(1, Qt::Horizontal, tr("Quantidade"));
    modeloProdutos->setHeaderData(2, Qt::Horizontal, tr("Descrição"));
    modeloProdutos->setHeaderData(3, Qt::Horizontal, tr("Preço"));
    modeloProdutos->setHeaderData(4, Qt::Horizontal, tr("Código de Barras"));
    modeloProdutos->setHeaderData(5, Qt::Horizontal, tr("NF"));

    CustomDelegate *delegate = new CustomDelegate(this);
    ui->Tview_Produtos->setItemDelegateForColumn(1,delegate);
    ui->Tview_Produtos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    db.close();
    modeloSelecionados->setHorizontalHeaderItem(0, new QStandardItem("ID Produto"));
    modeloSelecionados->setHorizontalHeaderItem(1, new QStandardItem("Quantidade Vendida"));
    modeloSelecionados->setHorizontalHeaderItem(2, new QStandardItem("Descrição"));
    modeloSelecionados->setHorizontalHeaderItem(3, new QStandardItem("Preço Unitário Vendido"));
    ui->Tview_ProdutosSelecionados->setModel(modeloSelecionados);
    // Selecionar a primeira linha da tabela
    QModelIndex firstIndex = modeloProdutos->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
    // Obter o modelo de seleção da tabela
    QItemSelectionModel *selectionModel = ui->Tview_ProdutosSelecionados->selectionModel();
    // Conectar o sinal de seleção ao slot personalizado
    connect(selectionModel, &QItemSelectionModel::selectionChanged,this, &venda::handleSelectionChange);
    // ajustar tamanho colunas
    // coluna descricao
    ui->Tview_Produtos->setColumnWidth(2, 260);
    // coluna quantidade
    ui->Tview_Produtos->setColumnWidth(1, 85);
    // coluna quantidade vendida
    ui->Tview_ProdutosSelecionados->setColumnWidth(1, 180);
    // coluna descricao
    ui->Tview_ProdutosSelecionados->setColumnWidth(2, 250);

   // ui->Tview_ProdutosSelecionados->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    //ui->Tview_Produtos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // colocar a data atual no dateEdit
    ui->DateEdt_Venda->setDateTime(QDateTime::currentDateTime());

    // setar o foco no codigo de barras
    ui->Ledit_Barras->setFocus();

    // validadores para os campos
    QDoubleValidator *DoubleValidador = new QDoubleValidator(0.0, 9999.99, 2);
    ui->Ledit_Preco->setValidator(DoubleValidador);
    ui->Ledit_QuantVendido->setValidator(DoubleValidador);

    //actionMenu contextMenu
    actionMenuDeletarProd = new QAction(this);
    actionMenuDeletarProd->setText("Deletar Produto");
    deletar.addFile(":/QEstoqueLOja/amarok-cart-remove.svg");
    actionMenuDeletarProd->setIcon(deletar);
    connect(actionMenuDeletarProd,SIGNAL(triggered(bool)),this,SLOT(deletarProd()));

   // actionMenuDeletarProd->setIcon(janelaPrincipal->iconDelete);
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
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));
    QString idProduto = idVariant.toString();
    QString descProduto = descVariant.toString();
    // preco com notacao br
    QString precoProduto = portugues.toString(precoVariant.toFloat());
    // mostrar na tabela Selecionados
    modeloSelecionados->appendRow({new QStandardItem(idProduto), new QStandardItem("1"), new QStandardItem(descProduto), new QStandardItem(precoProduto)});
    // mostrar total
    ui->Lbl_Total->setText(Total());
}


void venda::handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected) {
    // Este slot é chamado sempre que a seleção na tabela muda
    Q_UNUSED(deselected);

    if (selected.indexes().isEmpty()) {
        qDebug() << "Nenhum registro selecionado.";
        db.close();
        return;
    }

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

    QString inputText = ui->Ledit_Pesquisa->text();
    QString normalizadoPesquisa = janelaPrincipal->normalizeText(inputText);

    // Dividir a string em palavras usando split por espaços em branco
    QStringList palavras = normalizadoPesquisa.split(" ", Qt::SkipEmptyParts);

    // Exibir as palavras separadas no console (opcional)
    // qDebug() << "Palavras separadas:";
    // for (const QString& palavra : palavras) {
    //     qDebug() << palavra;
    // }

    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados. Botão Pesquisar.";
        return;
    }



    // Construir consulta SQL dinâmica
    QString sql = "SELECT * FROM produtos WHERE ";
    QStringList conditions;
    if (palavras.length() > 1){
        for (const QString &palavra : palavras) {
            conditions << QString("descricao LIKE '%%1%'").arg(palavra);

        }

        sql += conditions.join(" AND ");

    }else{
        sql += "descricao LIKE '%" + normalizadoPesquisa + "%'  OR codigo_barras LIKE '%" + normalizadoPesquisa + "%'";
    }
    sql += " ORDER BY id DESC";

    // Executar a consulta
    modeloProdutos->setQuery(sql, db);


    // Mostrar na tableview a consulta
    // CustomDelegate *delegate = new CustomDelegate(this);
    // ui->Tview_Produtos->setItemDelegate(delegate);
    ui->Tview_Produtos->setModel(modeloProdutos);

    db.close();
}


void venda::on_Ledit_QuantVendido_textChanged(const QString &arg1)
{
    // slot sempre que a quantidade for alterada, mudar o produto selecionado

    // Verificar se há algum registro selecionado na tabela
    if(ui->Tview_ProdutosSelecionados->selectionModel()->selectedIndexes().isEmpty()) {
        // Se não houver nenhum registro selecionado, sair da função
        return;
    }
    // pegar o produto selecionado
    QItemSelectionModel *selectionModel = ui->Tview_ProdutosSelecionados->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    int registroSelecionado = selectedIndex.row();
    // pegar o valor no line edit
    QString quantidade = ui->Ledit_QuantVendido->text();
    //
    QModelIndex quantidadeIndice = modeloSelecionados->index(registroSelecionado, 1);
    modeloSelecionados->setData(quantidadeIndice, quantidade);
    // mostrar total
    ui->Lbl_Total->setText(Total());
}


void venda::on_Ledit_Preco_textChanged(const QString &arg1)
{
    // slot sempre que o preço for alterado, mudar o produto selecionado

    // Verificar se há algum registro selecionado na tabela
    if(ui->Tview_ProdutosSelecionados->selectionModel()->selectedIndexes().isEmpty()) {
        // Se não houver nenhum registro selecionado, sair da função
        return;
    }
    // pegar o produto selecionado
    QItemSelectionModel *selectionModel = ui->Tview_ProdutosSelecionados->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    int registroSelecionado = selectedIndex.row();
    // pegar o valor no line edit
    QString preco = ui->Ledit_Preco->text();
    //
    QModelIndex precoIndice = modeloSelecionados->index(registroSelecionado, 3);
    modeloSelecionados->setData(precoIndice, preco);
    // mostrar total
    ui->Lbl_Total->setText(Total());
}


void venda::on_Ledit_Barras_returnPressed()
{
    // código de barras inserido
    // verificar se o codigo de barras ja existe
    QString barrasProduto = ui->Ledit_Barras->text();
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao enviar.";
    }
    QSqlQuery query;

    query.prepare("SELECT COUNT(*) FROM produtos WHERE codigo_barras = :codigoBarras");
    query.bindValue(":codigoBarras", barrasProduto);
    if (!query.exec()) {
        qDebug() << "Erro na consulta: contagem codigo barras";
    }
    query.next();
    bool barrasExiste = query.value(0).toInt() > 0 && barrasProduto != "";
    qDebug() << barrasProduto;
    if(barrasExiste){
        // o código existe
        QSqlQuery query;

        query.prepare("SELECT * FROM produtos WHERE codigo_barras = :codigoBarras");
        query.bindValue(":codigoBarras", barrasProduto);
        if (!query.exec()) {
            qDebug() << "Erro na consulta: contagem codigo barras";
        }
        query.next();
        QString idBarras = query.value(0).toString();
        QString descBarras = query.value(2).toString();
        // preco na notacao br
        QString precoBarras = portugues.toString(query.value(3).toFloat());
        qDebug() << idBarras;

        // mostrar na tabela Selecionados
        modeloSelecionados->appendRow({new QStandardItem(idBarras), new QStandardItem("1"), new QStandardItem(descBarras), new QStandardItem(precoBarras)});

        ui->Ledit_Barras->clear();

        // mostrar total
        ui->Lbl_Total->setText(Total());

    }
    else{
        // o código não existe
        QMessageBox::warning(this, "Erro", "Esse código de barras não foi registrado ainda.");
    }
}


void venda::on_Btn_Aceitar_clicked()
{
    // pegar os valores da tabela dos produtos selecionados
    QList<QList<QVariant>> rowDataList;
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row){
        QList<QVariant> rowData;
        for (int col = 0; col < modeloSelecionados->columnCount(); col++){
            QModelIndex index = modeloSelecionados->index(row, col);
            rowData.append(modeloSelecionados->data(index));
        }
        rowDataList.append(rowData);
    }
    qDebug() << rowDataList;
    // validar preço e quantidade
    bool erro = false;
    for (const QList<QVariant> &rowdata : rowDataList) {
        QString preco = rowdata[3].toString();
        QString quant = rowdata[1].toString();

        // Converta o texto para um número
        bool conversionOk, conversionOkQuant;
        double price = portugues.toDouble(preco, &conversionOk);
        int quantINT = portugues.toFloat(quant, &conversionOkQuant);

        // Verifique se a conversão foi bem-sucedida e se o preço é maior que zero
        if (!(conversionOk && price >= 0) || !(conversionOkQuant && quantINT > 0))
        {
            erro = true;
        }
    }
    qDebug() << "Erro = ";
    qDebug() << erro;
    if (!erro){
        // se nao tiver erro na validaçao, prossiga
        QString cliente = ui->Ledit_Cliente->text();
        QString data =  portugues.toString(ui->DateEdt_Venda->dateTime(), "dd-MM-yyyy hh:mm:ss");

        pagamentoVenda *pagamento = new pagamentoVenda(rowDataList, this, Total(), cliente, data);
        pagamento->setWindowModality(Qt::ApplicationModal);
        pagamento->show();
    }
    else {
        // Exiba uma mensagem de erro se o preço ou a quantidade não for válido
        QMessageBox::warning(this, "Erro", "Por favor, insira preços e/ou quantidades válidas.");
    }

}

QString venda::Total(){
    // Obtendo os dados da tabela e calculando o valor total da venda
    double totalValue = 0.0;
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        float quantidade = portugues.toFloat(modeloSelecionados->data(modeloSelecionados->index(row, 1)).toString());  // Coluna de quantidade
        double preco = portugues.toDouble(modeloSelecionados->data(modeloSelecionados->index(row, 3)).toString());  // Coluna de preço
        totalValue += quantidade * preco;
    }
    // total notacao br
    return portugues.toString(totalValue, 'f', 2);
}

void venda::on_Ledit_Pesquisa_textChanged(const QString &arg1)
{
    ui->Btn_Pesquisa->click();
}


void venda::on_Tview_ProdutosSelecionados_customContextMenuRequested(const QPoint &pos)
{
    if(!ui->Tview_ProdutosSelecionados->currentIndex().isValid())
        return;
    QMenu menu(this);

    menu.addAction(actionMenuDeletarProd);



    menu.exec(ui->Tview_ProdutosSelecionados->viewport()->mapToGlobal(pos));
}
void venda::deletarProd(){
    modeloSelecionados->removeRow(ui->Tview_ProdutosSelecionados->currentIndex().row());
    ui->Lbl_Total->setText(Total());
}

