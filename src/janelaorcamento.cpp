#include "janelaorcamento.h"
#include "ui_janelaorcamento.h"
#include <QDebug>
#include <QMessageBox>
#include <QCompleter>
#include <QStringListModel>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include "delegateprecovalidate.h"
#include "delegatelockcol.h"
#include "delegatequant.h"
#include "subclass/produtotableview.h"
#include "inserircliente.h"
#include <qtrpt.h>

JanelaOrcamento::JanelaOrcamento(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::JanelaOrcamento)
{
    ui->setupUi(this);

    Config_service *confServ = new Config_service(this);
    configDTO = confServ->carregarTudo();

    configurarOrcamentoEstoque();
}

JanelaOrcamento::~JanelaOrcamento()
{
    delete ui;
}

void JanelaOrcamento::deletarProd()
{
    modeloSelecionados->removeRow(ui->Tview_ProdutosSelec->currentIndex().row());
    ui->Lbl_TotalGeral->setText(totalGeral());
}

int JanelaOrcamento::validarCliente(bool mostrarMensagens)
{
    auto resultado = clienteService.validarClienteTexto(ui->Ledit_Cliente->text().trimmed());

    if (!resultado.ok) {
        if (mostrarMensagens) {
            QMessageBox::warning(this, "Cliente", resultado.msg);
            ui->Ledit_Cliente->setFocus();
        }
        return -1;
    }

    if (!resultado.nomeCorrigido.isEmpty())
        ui->Ledit_Cliente->setText(resultado.nomeCorrigido);

    return static_cast<int>(resultado.clienteId);
}

void JanelaOrcamento::atualizarListaCliente()
{
    clientesComId = clienteService.listarClientesParaCompleter();

    QCompleter *completer = ui->Ledit_Cliente->completer();
    if (completer) {
        QStringListModel *model = qobject_cast<QStringListModel*>(completer->model());
        if (model)
            model->setStringList(clientesComId);
    }
}

void JanelaOrcamento::configurarOrcamentoEstoque()
{
    modeloSelecionados->setHorizontalHeaderItem(0, new QStandardItem("ID Produto"));
    modeloSelecionados->setHorizontalHeaderItem(1, new QStandardItem("Quantidade Vendida"));
    modeloSelecionados->setHorizontalHeaderItem(2, new QStandardItem("Descrição"));
    modeloSelecionados->setHorizontalHeaderItem(3, new QStandardItem("Preço Unitário Vendido"));
    modeloSelecionados->setHorizontalHeaderItem(4, new QStandardItem("Total"));

    ui->Tview_ProdutosSelec->setModel(modeloSelecionados);
    ui->Tview_ProdutosSelec->setColumnWidth(0, 70);
    ui->Tview_ProdutosSelec->setColumnWidth(1, 130);
    ui->Tview_ProdutosSelec->setColumnWidth(2, 300);
    ui->Tview_ProdutosSelec->setColumnWidth(3, 150);

    DelegatePrecoValidate *validatePreco = new DelegatePrecoValidate(this);
    ui->Tview_ProdutosSelec->setItemDelegateForColumn(3, validatePreco);
    DelegateLockCol *delegateLockCol = new DelegateLockCol(0, this);
    ui->Tview_ProdutosSelec->setItemDelegateForColumn(0, delegateLockCol);
    DelegateLockCol *delegateLockCol2 = new DelegateLockCol(2, this);
    DelegateLockCol *delegateLockCol3 = new DelegateLockCol(4, this);
    ui->Tview_ProdutosSelec->setItemDelegateForColumn(4, delegateLockCol3);
    ui->Tview_ProdutosSelec->setItemDelegateForColumn(2, delegateLockCol2);
    DelegateQuant *delegateQuant = new DelegateQuant(this);
    ui->Tview_ProdutosSelec->setItemDelegateForColumn(1, delegateQuant);

    connect(modeloSelecionados, &QStandardItemModel::itemChanged, this, [=]() {
        ui->Lbl_TotalGeral->setText(totalGeral());
        atualizarTotalProduto();
    });

    QCompleter *completer = new QCompleter(this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);

    atualizarListaCliente();

    if (!clientesComId.isEmpty()) {
        ui->Ledit_Cliente->setText(clientesComId.first());

        QString primeiroCliente = clientesComId.first();
        int posFinalNome = primeiroCliente.indexOf(" (ID:");
        if (posFinalNome != -1)
            ui->Ledit_Cliente->setSelection(0, posFinalNome);
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
        validarCliente(true);
    });

    actionMenuDeletarProd = new QAction(this);
    actionMenuDeletarProd->setText("Deletar Produto");
    connect(actionMenuDeletarProd, SIGNAL(triggered(bool)), this, SLOT(deletarProd()));

    ui->Tview_ProdutosSelec->setContextMenuPolicy(Qt::CustomContextMenu);
}

QString JanelaOrcamento::totalGeral()
{
    double totalValue = 0.0;
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        float quantidade = portugues.toFloat(modeloSelecionados->data(modeloSelecionados->index(row, 1)).toString());
        double preco = portugues.toDouble(modeloSelecionados->data(modeloSelecionados->index(row, 3)).toString());
        totalValue += quantidade * preco;
    }
    return portugues.toString(totalValue, 'f', 2);
}

void JanelaOrcamento::atualizarTotalProduto()
{
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        float quantidade = portugues.toFloat(
            modeloSelecionados->data(modeloSelecionados->index(row, 1)).toString());
        double preco = portugues.toDouble(
            modeloSelecionados->data(modeloSelecionados->index(row, 3)).toString());

        modeloSelecionados->setData(
            modeloSelecionados->index(row, 4),
            portugues.toString(quantidade * preco, 'f', 2));
    }
}

void JanelaOrcamento::on_Btn_AddProd_clicked()
{
    ProdutoTableView *ptv = qobject_cast<ProdutoTableView*>(ui->Tview_ProdutosOrcamento);
    QSqlQueryModel *modelo = ptv->getModel();

    QItemSelectionModel *selectionModel = ui->Tview_ProdutosOrcamento->selectionModel();

    if (!selectionModel || selectionModel->selectedIndexes().isEmpty()) {
        QMessageBox::warning(this, "Erro", "Nenhum produto selecionado!");
        return;
    }

    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QString idProduto  = modelo->data(modelo->index(selectedIndex.row(), 0)).toString();
    QString descProduto = modelo->data(modelo->index(selectedIndex.row(), 2)).toString();
    float precoProduto  = modelo->data(modelo->index(selectedIndex.row(), 3)).toFloat();

    QStandardItem *itemPreco = new QStandardItem();
    itemPreco->setData(precoProduto, Qt::EditRole);
    itemPreco->setText(portugues.toString(precoProduto, 'f', 2));

    QStandardItem *itemTotal = new QStandardItem();
    itemTotal->setData(precoProduto, Qt::EditRole);
    itemTotal->setText(portugues.toString(precoProduto, 'f', 2));

    modeloSelecionados->appendRow({
        new QStandardItem(idProduto),
        new QStandardItem("1"),
        new QStandardItem(descProduto),
        itemPreco,
        itemTotal
    });

    ui->Lbl_TotalGeral->setText(totalGeral());
}

void JanelaOrcamento::on_Ledit_PesquisaProduto_textChanged(const QString &arg1)
{
    ProdutoTableView *ptv = qobject_cast<ProdutoTableView*>(ui->Tview_ProdutosOrcamento);
    produtoService.pesquisar(arg1, ptv->getModel());
    ui->Tview_ProdutosOrcamento->setModel(ptv->getModel());
}

void JanelaOrcamento::on_Btn_Terminar_clicked()
{
    int idCliente = validarCliente(true);
    if (idCliente < 0)
        return;

    auto [nome, id] = clienteService.extrairNomeId(ui->Ledit_Cliente->text());

    QList<QList<QVariant>> rowDataList;
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        QList<QVariant> rowData;
        for (int col = 0; col < modeloSelecionados->columnCount(); col++)
            rowData.append(modeloSelecionados->data(modeloSelecionados->index(row, col)));
        rowDataList.append(rowData);
    }

    QString data = portugues.toString(QDateTime::currentDateTime(), "dd-MM-yyyy hh:mm:ss");

    ClienteDTO cliente = clienteService.getClienteByID(idCliente);

    QString reportPath;
    for (const QString &basePath : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        QString candidate = basePath + "/QEstoqueLoja/reports/orcamentoReport.xml";
        if (QFileInfo::exists(candidate)) {
            reportPath = candidate;
            break;
        }
    }

    QtRPT *report = new QtRPT(nullptr);
    report->loadReport(reportPath);

    connect(report, &QtRPT::setDSInfo, [&](DataSetInfo &dsinfo) {
        dsinfo.recordCount = rowDataList.size();
    });

    connect(report, &QtRPT::setValue, [&](const int recno, const QString paramname, QVariant &paramvalue, const int reportpage) {
        Q_UNUSED(reportpage);

        if      (paramname == "nomeEmpresa")      paramvalue = configDTO.nomeEmpresa;
        else if (paramname == "endereco")          paramvalue = configDTO.enderecoEmpresa;
        else if (paramname == "cidade")            paramvalue = configDTO.cidadeEmpresa;
        else if (paramname == "estado")            paramvalue = configDTO.estadoEmpresa;
        else if (paramname == "email")             paramvalue = configDTO.emailEmpresa;
        else if (paramname == "cnpj")              paramvalue = configDTO.cnpjEmpresa;
        else if (paramname == "telefone")          paramvalue = configDTO.telefoneEmpresa;
        else if (paramname == "obs")               paramvalue = ui->Tedit_Obs->toPlainText();
        else if (paramname == "total_geral")       paramvalue = totalGeral();
        else if (paramname == "nome_cliente")      paramvalue = nome;
        else if (paramname == "endereco_cliente")  paramvalue = cliente.endereco;
        else if (paramname == "cpf_cliente")       paramvalue = cliente.cpf;
        else if (paramname == "email_cliente")     paramvalue = cliente.email;
        else if (paramname == "telefone_cliente")  paramvalue = cliente.telefone;
        else if (paramname == "data")              paramvalue = data;

        if (recno < rowDataList.size()) {
            const auto &rowData = rowDataList.at(recno);
            if      (paramname == "id_produto")    paramvalue = rowData.at(0);
            else if (paramname == "nome_produto")  paramvalue = rowData.at(2);
            else if (paramname == "quantidade")    paramvalue = rowData.at(1);
            else if (paramname == "preco_unitario") paramvalue = rowData.at(3);
            else if (paramname == "subtotal")      paramvalue = rowData.at(4);
        }
    });

    connect(report, &QtRPT::setValueImage, [&](const int recno, const QString paramname, QImage &paramvalue, const int reportpage) {
        Q_UNUSED(reportpage);
        Q_UNUSED(recno);

        if (paramname == "imgLogo") {
            QString caminhoCompleto = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                      + "/imagens/" + QFileInfo(configDTO.logoPathEmpresa).fileName();
            if (!QFile::exists(caminhoCompleto)) {
                qDebug() << "Erro: Arquivo não encontrado:" << caminhoCompleto;
                return;
            }
            QImage img(caminhoCompleto);
            if (img.isNull())
                qDebug() << "Erro ao carregar imagem:" << caminhoCompleto;
            else
                paramvalue = img;
        }
    });

    report->printExec();
}

void JanelaOrcamento::selecionarClienteNovo()
{
    atualizarListaCliente();
    if (!clientesComId.isEmpty()) {
        ui->Ledit_Cliente->setText(clientesComId.last());
        int posFinalNome = clientesComId.last().indexOf(" (ID:");
        if (posFinalNome != -1)
            ui->Ledit_Cliente->setSelection(0, posFinalNome);
    }
}

void JanelaOrcamento::on_Btn_NovoCliente_clicked()
{
    InserirCliente *inserirCliente = new InserirCliente;
    inserirCliente->setWindowModality(Qt::ApplicationModal);
    connect(inserirCliente, &InserirCliente::clienteInserido, this, &JanelaOrcamento::selecionarClienteNovo);
    inserirCliente->show();
}

void JanelaOrcamento::on_Tview_ProdutosSelec_customContextMenuRequested(const QPoint &pos)
{
    if (!ui->Tview_ProdutosSelec->currentIndex().isValid())
        return;
    QMenu menu(this);
    menu.addAction(actionMenuDeletarProd);
    menu.exec(ui->Tview_ProdutosSelec->viewport()->mapToGlobal(pos));
}
