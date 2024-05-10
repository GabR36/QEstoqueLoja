#include "venda.h"
#include "ui_venda.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QVector>
#include <QMessageBox>
#include "pagamento.h"


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

    // setar o foco no codigo de barras
    ui->Ledit_Barras->setFocus();
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
    QString precoProduto = precoVariant.toString();
    // mostrar na tabela Selecionados
    modeloSelecionados.appendRow({new QStandardItem(idProduto), new QStandardItem("1"), new QStandardItem(descProduto), new QStandardItem(precoProduto)});
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
    // mostrar total
    ui->Lbl_Total->setText(Total());
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
    QModelIndex quantidadeIndice = modeloSelecionados.index(registroSelecionado, 1);
    modeloSelecionados.setData(quantidadeIndice, quantidade);
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
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
    QModelIndex precoIndice = modeloSelecionados.index(registroSelecionado, 3);
    modeloSelecionados.setData(precoIndice, preco);
    ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);
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
        QString precoBarras = query.value(3).toString();
        qDebug() << idBarras;

        // mostrar na tabela Selecionados
        modeloSelecionados.appendRow({new QStandardItem(idBarras), new QStandardItem("1"), new QStandardItem(descBarras), new QStandardItem(precoBarras)});
        ui->Tview_ProdutosSelecionados->setModel(&modeloSelecionados);

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
    for (int row = 0; row < modeloSelecionados.rowCount(); ++row){
        QList<QVariant> rowData;
        for (int col = 0; col < modeloSelecionados.columnCount(); col++){
            QModelIndex index = modeloSelecionados.index(row, col);
            rowData.append(modeloSelecionados.data(index));
        }
        rowDataList.append(rowData);
    }
    qDebug() << rowDataList;
    // validar preço e quantidade
    bool erro = false;
    for (const QList<QVariant> &rowdata : rowDataList) {
        // Substitua ',' por '.' se necessário
        QString preco = rowdata[3].toString();
        QString quant = rowdata[1].toString();
        preco.replace(',', '.');

        // Converta o texto para um número
        bool conversionOk, conversionOkQuant;
        double price = preco.toDouble(&conversionOk);
        int quantINT = quant.toInt(&conversionOkQuant);

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
        QString data = ui->DateEdt_Venda->dateTime().toString("dd-MM-yyyy HH:mm:ss");

        pagamento *JanelaPagamento = new pagamento(Total(), cliente, data);
        JanelaPagamento->janelaVenda = this;
        JanelaPagamento->rowDataList = rowDataList;
        JanelaPagamento->setWindowModality(Qt::ApplicationModal);
        JanelaPagamento->show();    
    }
    else {
        // Exiba uma mensagem de erro se o preço ou a quantidade não for válido
        QMessageBox::warning(this, "Erro", "Por favor, insira preços e/ou quantidades válidas.");
    }

}

QString venda::Total(){
    // Obtendo os dados da tabela e calculando o valor total da venda
    double totalValue = 0.0;
    for (int row = 0; row < modeloSelecionados.rowCount(); ++row) {
        int quantidade = modeloSelecionados.data(modeloSelecionados.index(row, 1)).toInt();  // Coluna de quantidade
        double preco = modeloSelecionados.data(modeloSelecionados.index(row, 3)).toDouble();  // Coluna de preço
        totalValue += quantidade * preco;
    }
    return QString::number(totalValue);
}
