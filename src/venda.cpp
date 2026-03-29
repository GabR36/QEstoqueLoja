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
#include "delegateprecof2.h"
#include "delegateprecovalidate.h"
#include "delegatelockcol.h"
#include "delegatequant.h"
#include <QCompleter>
#include <QStringListModel>
#include "inserircliente.h"
#include "infojanelaprod.h"
#include "../services/Produto_service.h"
#include "services/config_service.h"

venda::venda(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::venda)
{
    ui->setupUi(this);

    prodServ.listarProdutos(modeloProdutos);
    ui->Tview_Produtos->setModel(modeloProdutos);
    modeloProdutos->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modeloProdutos->setHeaderData(1, Qt::Horizontal, tr("Quantidade"));
    modeloProdutos->setHeaderData(2, Qt::Horizontal, tr("Descrição"));
    modeloProdutos->setHeaderData(3, Qt::Horizontal, tr("Preço"));
    modeloProdutos->setHeaderData(4, Qt::Horizontal, tr("Código de Barras"));
    modeloProdutos->setHeaderData(5, Qt::Horizontal, tr("NF"));

    // -- delegates
    CustomDelegate *delegateVermelho = new CustomDelegate(this);
    ui->Tview_Produtos->setItemDelegateForColumn(1,delegateVermelho);
    //ui->Tview_Produtos->setItemDelegateForColumn(3, delegatePreco);
   // ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(3, delegatePreco);
    DelegatePrecoValidate *validatePreco = new DelegatePrecoValidate(this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(3,validatePreco);
    DelegateLockCol *delegateLockCol = new DelegateLockCol(0,this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(0,delegateLockCol);
    DelegateLockCol *delegateLockCol2 = new DelegateLockCol(2,this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(2,delegateLockCol2);
    DelegateQuant *delegateQuant = new DelegateQuant(this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(1,delegateQuant);
    DelegateLockCol *delegateLockCol3 = new DelegateLockCol(4,this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(4,delegateLockCol3);


    ui->Tview_Produtos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");

    modeloSelecionados->setHorizontalHeaderItem(0, new QStandardItem("ID Produto"));
    modeloSelecionados->setHorizontalHeaderItem(1, new QStandardItem("Quantidade Vendida"));
    modeloSelecionados->setHorizontalHeaderItem(2, new QStandardItem("Descrição"));
    modeloSelecionados->setHorizontalHeaderItem(3, new QStandardItem("Preço Unitário Vendido"));
    modeloSelecionados->setHorizontalHeaderItem(4, new QStandardItem("Total"));
    ui->Tview_ProdutosSelecionados->setModel(modeloSelecionados);
    // Selecionar a primeira linha da tabela
    QModelIndex firstIndex = modeloProdutos->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
    // Obter o modelo de seleção da tabela
    QItemSelectionModel *selectionModel = ui->Tview_ProdutosSelecionados->selectionModel();
    // Conectar o sinal de seleção ao slot personalizado
    connect(selectionModel, &QItemSelectionModel::selectionChanged,this, &venda::handleSelectionChange);
    QItemSelectionModel *selectionModelProdutos = ui->Tview_Produtos->selectionModel();
    connect(selectionModelProdutos, &QItemSelectionModel::selectionChanged, this,
            &venda::handleSelectionChangeProdutos);

    // ajustar tamanho colunas
    // coluna descricao
    ui->Tview_Produtos->setColumnWidth(2, 260);
    // coluna quantidade
    ui->Tview_Produtos->setColumnWidth(1, 85);


    ui->Tview_ProdutosSelecionados->setColumnWidth(0, 70);
    // coluna quantidade vendida
    ui->Tview_ProdutosSelecionados->setColumnWidth(1, 130);
    // coluna descricao
    ui->Tview_ProdutosSelecionados->setColumnWidth(2, 300);
    ui->Tview_ProdutosSelecionados->setColumnWidth(3, 160);
    ui->Tview_ProdutosSelecionados->setColumnWidth(4, 200);

    // colocar a data atual no dateEdit
    ui->DateEdt_Venda->setDateTime(QDateTime::currentDateTime());

    // setar o foco no codigo de barras

    //actionMenu contextMenu
    actionMenuDeletarProd = new QAction(this);
    actionMenuDeletarProd->setText("Deletar Produto");
    deletar.addFile(":/QEstoqueLOja/amarok-cart-remove.svg");
    actionMenuDeletarProd->setIcon(deletar);
    connect(actionMenuDeletarProd,SIGNAL(triggered(bool)),this,SLOT(deletarProd()));

    //torna a tabela editavel
    ui->Tview_ProdutosSelecionados->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

    ui->Btn_SelecionarProduto->setEnabled(false);

    connect(modeloSelecionados, &QStandardItemModel::itemChanged, this, [=]() {
        ui->Lbl_Total->setText(Total());
        atualizarTotalProduto();
    });

    QCompleter *completer = new QCompleter(this);
    completer->setCaseSensitivity(Qt::CaseInsensitive); // Ignorar maiúsculas e minúsculas
    completer->setFilterMode(Qt::MatchContains); // Sugestões que contêm o texto digitado

    atualizarListaCliente();

    if (!clientesComId.isEmpty()) {
        // Define o primeiro item da lista como texto do QLineEdit
        ui->Ledit_Cliente->setText(clientesComId.first());

        QString primeiroCliente = clientesComId.first();
        int posInicioNome = 0;
        int posFinalNome = primeiroCliente.indexOf(" (ID:"); // Encontra onde começa o ID

        if (posFinalNome != -1) {
            // Seleciona apenas o nome (sem o ID)
            ui->Ledit_Cliente->setSelection(posInicioNome, posFinalNome);
        }
    }

    QStringListModel *model = new QStringListModel(clientesComId, this);
    completer->setModel(model);
    ui->Ledit_Cliente->setCompleter(completer);


    connect(ui->Ledit_Cliente, &QLineEdit::textEdited, this, [=]() {
            completer->complete();
    });
    connect(ui->Ledit_Cliente, &QLineEdit::cursorPositionChanged, this, [=]() {
            completer->complete();
    });

    connect(ui->Ledit_Cliente, &QLineEdit::editingFinished, this, [=]() {
        validarCliente(true); // Mostra mensagens para o usuário
    });


    Config_service *confServ = new Config_service(this);

    configDTO = confServ->carregarTudo();
    bool tipoAmb = configDTO.tpAmbFiscal;
    bool emitirNf = configDTO.emitNfFiscal;

    if (tipoAmb == 1 && emitirNf == 1) {
        ui->Lbl_TpAmb->setText("Ambiente: Produção");
        ui->Lbl_TpAmb->setStyleSheet("color: white; background-color: green; font-weight: bold; padding: 4px; border-radius: 5px;");
    } else if(emitirNf == 1){
        ui->Lbl_TpAmb->setText("Ambiente: Homologação");
        ui->Lbl_TpAmb->setStyleSheet("color: white; background-color: orange; font-weight: bold; padding: 4px; border-radius: 5px;");
    }
    connect(ui->Tview_Produtos, &QTableView::doubleClicked,
            this, &venda::verProd);
    ui->Ledit_Pesquisa->setFocus();
}

void venda::atualizarTotalProduto() {
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        double totalproduto = 0.0;

        float quantidade = portugues.toFloat(
            modeloSelecionados->data(modeloSelecionados->index(row, 1)).toString()
            ); // Coluna de quantidade

        double preco = portugues.toDouble(
            modeloSelecionados->data(modeloSelecionados->index(row, 3)).toString()
            ); // Coluna de preço

        totalproduto = quantidade * preco;

        // Atualiza o valor na coluna 4
        modeloSelecionados->setData(
            modeloSelecionados->index(row, 4),
            portugues.toString(totalproduto, 'f', 2) // 2 casas decimais
            );
    }
}

void venda::atualizarListaCliente()
{
    clientesComId = cliServ.listarClientesParaCompleter();

    QCompleter *completer = ui->Ledit_Cliente->completer();

    if (completer) {
        QStringListModel *model =
            qobject_cast<QStringListModel*>(completer->model());

        if (model) {
            model->setStringList(clientesComId);
        }
    }
}


venda::~venda()
{
    delete ui;
}
qlonglong venda::validarCliente(bool mostrarMensagens)
{
    auto resultado =
        cliServ.validarClienteTexto(ui->Ledit_Cliente->text());

    if(!resultado.ok)
    {
        if(mostrarMensagens)
        {
            switch(resultado.erro)
            {
            case ClienteErro::CampoVazio:
                QMessageBox::warning(this,"Cliente",
                                     "Por favor informe o cliente.");
                ui->Ledit_Cliente->setFocus();
                break;

            case ClienteErro::InsercaoInvalida:
                QMessageBox::warning(this,"Cliente",
                                     "Cliente não encontrado.");
                ui->Ledit_Cliente->clear();
                ui->Ledit_Cliente->setFocus();
                break;

            case ClienteErro::QuebraDeRegra:
                QMessageBox::warning(this,"Cliente",
                                     "Nome não corresponde ao ID.");
                ui->Ledit_Cliente->selectAll();
                ui->Ledit_Cliente->setFocus();
                break;

            default:
                QMessageBox::warning(this,"Erro",
                                     resultado.msg);
            }
        }

        return -1;
    }

    if(!resultado.nomeCorrigido.isEmpty())
        ui->Ledit_Cliente->setText(resultado.nomeCorrigido);

    return resultado.clienteId;
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

    float precoProduto = precoVariant.toFloat();
    // mostrar na tabela Selecionados
    QStandardItem *itemQuantidade = new QStandardItem("1");
    //itemQuantidade->setEditable(true);
    QStandardItem *itemPreco = new QStandardItem();
    itemPreco->setData(precoProduto, Qt::EditRole);  // valor bruto
    itemPreco->setText(portugues.toString(precoProduto, 'f', 2)); // texto formatado

    QStandardItem *itemTotal = new QStandardItem();
    itemTotal->setData(precoProduto, Qt::EditRole);
    itemTotal->setText(portugues.toString(precoProduto, 'f', 2));

    modeloSelecionados->appendRow({new QStandardItem(idProduto), itemQuantidade,
                                   new QStandardItem(descProduto), itemPreco, itemTotal});
    // mostrar total
    ui->Lbl_Total->setText(Total());
}

void venda::handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected) {
    // Este slot é chamado sempre que a seleção na tabela muda
    Q_UNUSED(deselected);
}

void venda::handleSelectionChangeProdutos(const QItemSelection &selected, const QItemSelection &deselected){
    if (selected.indexes().isEmpty()) {
        qDebug() << "Nenhum registro selecionado.";
        ui->Btn_SelecionarProduto->setEnabled(false);
        return;
    }else{
        ui->Btn_SelecionarProduto->setEnabled(true);

    }
}

void venda::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        ui->Ledit_Pesquisa->setFocus();  // Foca no QLineEdit quando F4 é pressionado
        ui->Ledit_Pesquisa->selectAll();
    }else if(event->key() == Qt::Key_Escape){
        ui->Btn_CancelarVenda->click();
    }else if(event->key() == Qt::Key_F4){
        ui->Tview_Produtos->setFocus();
    }else if(event->key() == Qt::Key_F2){
        ui->Ledit_Cliente->setFocus();
        ui->Ledit_Cliente->selectAll();
    }else if(event->key() == Qt::Key_F3){
        ui->DateEdt_Venda->setFocus();
    }else if(event->key() == Qt::Key_F9){
        ui->Tview_ProdutosSelecionados->setFocus();
    }
    else if(ui->Tview_Produtos->hasFocus() && (event->key() == Qt::Key_Return ||
                                                  event->key() == Qt::Key_Enter)){
        ui->Btn_SelecionarProduto->click();
    }else if(ui->Tview_ProdutosSelecionados->hasFocus() && (event->key() == Qt::Key_Delete)){
        deletarProd();
    }
    QWidget::keyPressEvent(event);
    // Chama a implementação base
}

void venda::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    ui->Ledit_Cliente->setReadOnly(false); // Permite edição ao focar
}

void venda::on_Btn_Pesquisa_clicked()
{
    QString inputText = ui->Ledit_Pesquisa->text();

    prodServ.pesquisar(inputText, modeloProdutos);

    if (!modeloProdutos) {
        QMessageBox::warning(this, "Erro", "Erro ao realizar a pesquisa.");
        return;
    }
}

QList<ProdutoVendidoDTO> venda::obterProdutosSelecionados()
{
    QList<ProdutoVendidoDTO> lista;

    QLocale brasil(QLocale::Portuguese, QLocale::Brazil);

    for (int row = 0; row < modeloSelecionados->rowCount(); ++row)
    {
        ProdutoVendidoDTO prod;

        prod.idProduto =
            modeloSelecionados->data(modeloSelecionados->index(row,0)).toLongLong();

        QString quantTexto =
            modeloSelecionados->data(modeloSelecionados->index(row,1)).toString();

        QString descricao =
            modeloSelecionados->data(modeloSelecionados->index(row,2)).toString();

        QString precoTexto =
            modeloSelecionados->data(modeloSelecionados->index(row,3)).toString();

        prod.quantidade = brasil.toDouble(quantTexto);
        prod.precoVendido = brasil.toDouble(precoTexto);

        prod.descricao = descricao;
        // prod.total = prod.quantidade * prod.precoUnitario;

        lista.append(prod);
    }

    return lista;
}

void venda::on_Btn_Aceitar_clicked()
{
    qlonglong idCliente = validarCliente(true);

    if (idCliente < 0)
        return;

    auto [nome, id] =
        cliServ.extrairNomeId(ui->Ledit_Cliente->text());

    QList<ProdutoVendidoDTO> produtos =
        obterProdutosSelecionados();

    QString data =
        portugues.toString(ui->DateEdt_Venda->dateTime(),
                           "dd-MM-yyyy hh:mm:ss");

    pagamentoVenda *pagamento =
        new pagamentoVenda(produtos,
                           Total(),
                           nome,
                           data,
                           idCliente);

    pagamento->setWindowModality(Qt::ApplicationModal);

    connect(pagamento,
            &pagamentoVenda::pagamentoConcluido,
            this,
            &venda::vendaConcluida);

    connect(pagamento,
            &pagamentoVenda::pagamentoConcluido,
            this,
            &venda::close);

    pagamento->show();
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

void venda::on_Ledit_Pesquisa_returnPressed()
{
    // código de barras inserido
    // verificar se o codigo de barras ja existe
    QString barrasProduto = ui->Ledit_Pesquisa->text();
    bool barrasExiste = prodServ.codigoBarrasExiste(barrasProduto);

    if(barrasExiste){
        // o código existe
        ProdutoDTO prodBarras = prodServ.getProdutoPeloCodBarras(barrasProduto);
        QString idBarras = QString::number(prodBarras.id);
        QString descBarras = prodBarras.descricao;
        // preco na notacao br
        QString precoBarras = portugues.toString(prodBarras.preco);
        qDebug() << idBarras;

        // mostrar na tabela Selecionados
        modeloSelecionados->appendRow({new QStandardItem(idBarras), new QStandardItem("1"), new QStandardItem(descBarras), new QStandardItem(precoBarras)});
        atualizarTotalProduto();
        ui->Ledit_Pesquisa->clear();

        // mostrar total
        ui->Lbl_Total->setText(Total());

    }
    else{
        // o código não existe
        QMessageBox::warning(this, "Erro", "Esse código de barras não foi registrado ainda.");
    }
}

void venda::on_Btn_CancelarVenda_clicked()
{
    this->close();
}
void venda::selecionarClienteNovo(){
    atualizarListaCliente();
    if (!clientesComId.isEmpty()) {
        // Define o ultimo item da lista como texto do QLineEdit

        ui->Ledit_Cliente->setText(clientesComId.last());

        QString ultimoCliente = clientesComId.last();
        int posInicioNome = 0;
        int posFinalNome = ultimoCliente.indexOf(" (ID:"); // Encontra onde começa o ID

        if (posFinalNome != -1) {
            // Seleciona apenas o nome (sem o ID)
            ui->Ledit_Cliente->setSelection(posInicioNome, posFinalNome);
        }
    }

}


void venda::on_Btn_NovoCliente_clicked()
{
    InserirCliente *inserirCliente = new InserirCliente;
    inserirCliente->setWindowModality(Qt::ApplicationModal);
    connect(inserirCliente, &InserirCliente::clienteInserido, this, &venda::selecionarClienteNovo);
    inserirCliente->show();
}
QString venda::getIdProdSelected(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex idIndex = ui->Tview_Produtos->model()->index(selectedRow, 0);

        QString id = ui->Tview_Produtos->model()->data(idIndex).toString();
        return id;
    }
}

void venda::verProd(){
    QString id = getIdProdSelected();
    InfoJanelaProd *janelaProd = new InfoJanelaProd(this, id);
    janelaProd->show();
}


