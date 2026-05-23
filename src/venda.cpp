#include "venda.h"
#include "ui_venda.h"
#include "customdelegate.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QVector>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QTimer>
#include "delegateprecof2.h"
#include "delegateprecovalidate.h"
#include "delegatelockcol.h"
#include "delegatequant.h"
#include <QCompleter>
#include <QStringListModel>
#include <QMenu>
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

    // delegates
    CustomDelegate *delegateVermelho = new CustomDelegate(this);
    ui->Tview_Produtos->setItemDelegateForColumn(1, delegateVermelho);
    DelegatePrecoValidate *validatePreco = new DelegatePrecoValidate(this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(3, validatePreco);
    DelegateLockCol *delegateLockCol = new DelegateLockCol(0, this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(0, delegateLockCol);
    DelegateLockCol *delegateLockCol2 = new DelegateLockCol(2, this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(2, delegateLockCol2);
    DelegateQuant *delegateQuant = new DelegateQuant(this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(1, delegateQuant);
    DelegateLockCol *delegateLockCol3 = new DelegateLockCol(4, this);
    ui->Tview_ProdutosSelecionados->setItemDelegateForColumn(4, delegateLockCol3);

    ui->Tview_Produtos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");

    modeloSelecionados->setHorizontalHeaderItem(0, new QStandardItem("ID Produto"));
    modeloSelecionados->setHorizontalHeaderItem(1, new QStandardItem("Quantidade Vendida"));
    modeloSelecionados->setHorizontalHeaderItem(2, new QStandardItem("Descrição"));
    modeloSelecionados->setHorizontalHeaderItem(3, new QStandardItem("Preço Unitário Vendido"));
    modeloSelecionados->setHorizontalHeaderItem(4, new QStandardItem("Total"));
    ui->Tview_ProdutosSelecionados->setModel(modeloSelecionados);

    QModelIndex firstIndex = modeloProdutos->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);

    QItemSelectionModel *selectionModel = ui->Tview_ProdutosSelecionados->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &venda::handleSelectionChange);
    QItemSelectionModel *selectionModelProdutos = ui->Tview_Produtos->selectionModel();
    connect(selectionModelProdutos, &QItemSelectionModel::selectionChanged, this,
            &venda::handleSelectionChangeProdutos);

    ui->Tview_Produtos->setColumnWidth(2, 260);
    ui->Tview_Produtos->setColumnWidth(1, 85);
    ui->Tview_ProdutosSelecionados->setColumnWidth(0, 70);
    ui->Tview_ProdutosSelecionados->setColumnWidth(1, 130);
    ui->Tview_ProdutosSelecionados->setColumnWidth(2, 300);
    ui->Tview_ProdutosSelecionados->setColumnWidth(3, 160);
    ui->Tview_ProdutosSelecionados->setColumnWidth(4, 200);

    ui->DateEdt_Venda->setDateTime(QDateTime::currentDateTime());

    actionMenuDeletarProd = new QAction(this);
    actionMenuDeletarProd->setText("Deletar Produto");
    deletar.addFile(":/QEstoqueLOja/amarok-cart-remove.svg");
    actionMenuDeletarProd->setIcon(deletar);
    connect(actionMenuDeletarProd, SIGNAL(triggered(bool)), this, SLOT(deletarProd()));

    ui->Tview_ProdutosSelecionados->setEditTriggers(
        QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

    ui->Btn_SelecionarProduto->setEnabled(false);

    connect(modeloSelecionados, &QStandardItemModel::itemChanged, this, [=]() {
        ui->Lbl_Total->setText(Total());
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

    connect(ui->Ledit_Cliente, &QLineEdit::textEdited, this, [=]() { completer->complete(); });
    connect(ui->Ledit_Cliente, &QLineEdit::cursorPositionChanged, this, [=]() { completer->complete(); });
    connect(ui->Ledit_Cliente, &QLineEdit::editingFinished, this, [=]() { validarCliente(true); });

    Config_service *confServ = new Config_service(this);
    configDTO = confServ->carregarTudo();
    bool tipoAmb = configDTO.tpAmbFiscal;
    bool emitirNf = configDTO.emitNfFiscal;

    if (tipoAmb == 1 && emitirNf == 1) {
        ui->Lbl_TpAmb->setText("Ambiente: Produção");
        ui->Lbl_TpAmb->setStyleSheet("color: white; background-color: green; font-weight: bold; padding: 4px; border-radius: 5px;");
    } else if (emitirNf == 1) {
        ui->Lbl_TpAmb->setText("Ambiente: Homologação");
        ui->Lbl_TpAmb->setStyleSheet("color: white; background-color: orange; font-weight: bold; padding: 4px; border-radius: 5px;");
    }

    connect(ui->Tview_Produtos, &QTableView::doubleClicked, this, &venda::verProd);

    // payment validators
    QDoubleValidator *validador = new QDoubleValidator(0.0, 9999.99, 2, this);
    ui->Ledit_Taxa->setValidator(validador);
    ui->Ledit_Recebido->setValidator(new QDoubleValidator(0.0, 99999.99, 2, this));
    ui->Ledit_Desconto->setValidator(new QDoubleValidator(0.0, 99999.99, 2, this));
    QIntValidator *validadorInt = new QIntValidator(this);
    ui->Ledit_NNF->setValidator(validadorInt);

    // step 4 só aparece quando NF está habilitado nas configurações
    ui->Lbl_Step4->setVisible(configDTO.emitNfFiscal);
    ui->lbl_sep3->setVisible(configDTO.emitNfFiscal);

    rascunhoTimer = new QTimer(this);
    rascunhoTimer->setSingleShot(true);
    rascunhoTimer->setInterval(400);
    connect(rascunhoTimer, &QTimer::timeout, this, &venda::salvarRascunho);

    auto agendarSalvar = [=]() { rascunhoTimer->start(); };

    // produtos
    connect(modeloSelecionados, &QStandardItemModel::itemChanged,  this, agendarSalvar);
    connect(modeloSelecionados, &QStandardItemModel::rowsInserted, this, agendarSalvar);
    connect(modeloSelecionados, &QStandardItemModel::rowsRemoved,  this, agendarSalvar);

    // cliente e data
    connect(ui->Ledit_Cliente,   &QLineEdit::editingFinished,  this, agendarSalvar);
    connect(ui->DateEdt_Venda,   &QDateTimeEdit::dateTimeChanged, this, agendarSalvar);

    // pagamento
    connect(ui->CBox_FormaPagamento, QOverload<int>::of(&QComboBox::currentIndexChanged), this, agendarSalvar);
    connect(ui->Ledit_Desconto,  &QLineEdit::textChanged, this, agendarSalvar);
    connect(ui->Ledit_Taxa,      &QLineEdit::textChanged, this, agendarSalvar);
    connect(ui->Ledit_Recebido,  &QLineEdit::textChanged, this, agendarSalvar);
    connect(ui->CheckPorcentagem, &QCheckBox::stateChanged, this, agendarSalvar);

    // nota fiscal
    connect(ui->CBox_ModeloEmit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, agendarSalvar);
    connect(ui->RadioBtn_EmitNfTodos,  &QRadioButton::toggled, this, agendarSalvar);
    connect(ui->RadioBtn_EmitNfApenas, &QRadioButton::toggled, this, agendarSalvar);

    irParaPagina(0);
    ui->Ledit_Pesquisa->setFocus();

    verificarRascunho();
}

// ─── Navigation ──────────────────────────────────────────────────────────────

void venda::irParaPagina(int pagina)
{
    ui->stack_Pages->setCurrentIndex(pagina);

    const QString styleAtivo   = "background-color: rgb(43,132,191); color: white; border-radius: 14px; font: 700 9pt \"Ubuntu\"; padding: 2px 12px;";
    const QString styleInativo = "background-color: rgba(195,215,235,60); color: rgba(195,215,235,160); border-radius: 14px; font: 700 9pt \"Ubuntu\"; padding: 2px 12px;";

    ui->Lbl_Step1->setStyleSheet(pagina == 0 ? styleAtivo : styleInativo);
    ui->Lbl_Step2->setStyleSheet(pagina == 1 ? styleAtivo : styleInativo);
    ui->Lbl_Step3->setStyleSheet(pagina == 2 ? styleAtivo : styleInativo);
    ui->Lbl_Step4->setStyleSheet(pagina == 3 ? styleAtivo : styleInativo);

    ui->Btn_Voltar->setVisible(pagina > 0);

    switch (pagina) {
    case 0:
        ui->Btn_Aceitar->setText("Próximo →");
        ui->Ledit_Pesquisa->setFocus();
        break;
    case 1:
        ui->Btn_Aceitar->setText("Próximo: Pagamento →");
        ui->DateEdt_Venda->setFocus();
        break;
    case 2:
        ui->Btn_Aceitar->setText(configDTO.emitNfFiscal ? "Próximo: Nota Fiscal →" : "✓ Confirmar Venda");
        ui->Ledit_Recebido->setFocus();
        break;
    case 3:
        ui->Btn_Aceitar->setText("Emitir e Confirmar →");
        ui->Ledit_NNF->setFocus();
        break;
    }
}

void venda::on_Btn_Aceitar_clicked()
{
    int pagina = ui->stack_Pages->currentIndex();

    if (pagina == 0) {
        if (modeloSelecionados->rowCount() == 0) {
            QMessageBox::warning(this, "Carrinho", "Adicione pelo menos um produto ao carrinho.");
            return;
        }
        ui->Lbl_ResumoTotalCliente->setText("R$ " + Total());
        ui->Lbl_ResumoItens->setText(QString::number(modeloSelecionados->rowCount()) + " itens");
        irParaPagina(1);

    } else if (pagina == 1) {
        idClienteAtual = validarCliente(true);
        if (idClienteAtual < 0)
            return;
        configurarPaginaPagamento();
        irParaPagina(2);

    } else if (pagina == 2) {
        if (configDTO.emitNfFiscal) {
            configurarPaginaNF();
            irParaPagina(3);
        } else {
            terminarPagamento();
        }
    } else if (pagina == 3) {
        terminarPagamento();
    }
}

void venda::on_Btn_Voltar_clicked()
{
    int pagina = ui->stack_Pages->currentIndex();
    if (pagina > 0)
        irParaPagina(pagina - 1);
}

// ─── Payment setup ────────────────────────────────────────────────────────────

void venda::configurarPaginaPagamento()
{
    auto [nome, id] = cliServ.extrairNomeId(ui->Ledit_Cliente->text());
    QString data = portugues.toString(ui->DateEdt_Venda->dateTime(), "dd-MM-yyyy hh:mm:ss");

    CLIENTE = cliServ.getClienteByID(idClienteAtual);

    ui->Lbl_ResumoTotal->setText(Total());
    ui->Lbl_ResumoCliente->setText(nome);
    ui->Lbl_ResumoData->setText(data);

    // defaults
    ui->Ledit_Recebido->setText(Total());
    ui->Lbl_Troco->setText("0");
    ui->Ledit_Desconto->setText("0");
    ui->Ledit_Taxa->setText("0");
    ui->Lbl_TotalTaxa->setText(Total());
    ui->Lbl_Total->setText(Total());

    if (temRascunhoPendente) {
        // aplica forma de pagamento salva e reconfigura visibilidade dos campos
        int idxForma = ui->CBox_FormaPagamento->findText(rascunhoPendente.formaPagamento);
        if (idxForma >= 0)
            ui->CBox_FormaPagamento->setCurrentIndex(idxForma);
        on_CBox_FormaPagamento_activated(ui->CBox_FormaPagamento->currentIndex());

        ui->CheckPorcentagem->setChecked(rascunhoPendente.descontoPorcentagem);
        ui->Ledit_Desconto->setText(rascunhoPendente.desconto);
        ui->Ledit_Taxa->setText(rascunhoPendente.taxa);
        ui->Ledit_Recebido->setText(rascunhoPendente.recebido);
    } else {
        // defaults para nova venda: dinheiro — hide taxa, show troco
        ui->lbl_taxa->hide();
        ui->Ledit_Taxa->hide();
        ui->label_2->show();
        ui->label_3->show();
        ui->Ledit_Recebido->show();
        ui->Lbl_Troco->show();
    }
}

void venda::configurarPaginaNF()
{
    if (temRascunhoPendente) {
        ui->CBox_ModeloEmit->setCurrentIndex(rascunhoPendente.modeloNf);
        if (rascunhoPendente.emitirTodos)
            ui->RadioBtn_EmitNfTodos->setChecked(true);
        else
            ui->RadioBtn_EmitNfApenas->setChecked(true);
        temRascunhoPendente = false;
    }

    int idx = ui->CBox_ModeloEmit->currentIndex();
    ModeloNota modelo = (idx == 1) ? ModeloNota::NFe : ModeloNota::NFCe;
    ui->Ledit_NNF->setText(QString::number(notaServ.getProximoNNF(configDTO.tpAmbFiscal, modelo)));

    // na primeira entrada com rascunho usa o CPF salvo; nas demais usa o do cliente
    if (!rascunhoPendente.cpfManual.isEmpty()) {
        ui->Ledit_CpfCnpjCliente->setText(rascunhoPendente.cpfManual);
        rascunhoPendente.cpfManual.clear();
    } else {
        ui->Ledit_CpfCnpjCliente->setText(CLIENTE.cpf);
    }

    bool emite = (idx != 2);
    ui->RadioBtn_EmitNfApenas->setVisible(emite);
    ui->RadioBtn_EmitNfTodos->setVisible(emite);
    ui->Lbl_NNF->setVisible(emite);
    ui->Ledit_NNF->setVisible(emite);
}

// ─── Payment form slots ───────────────────────────────────────────────────────

float venda::obterValorFinal(QString taxa, QString desconto)
{
    return (portugues.toFloat(Total()) - portugues.toFloat(desconto))
           * (1 + portugues.toFloat(taxa) / 100);
}

void venda::descontoTaxa()
{
    QString novaTaxa = ui->Ledit_Taxa->text();
    QString descontoInicial = ui->Ledit_Desconto->text();
    QString desconto;
    if (ui->CheckPorcentagem->isChecked())
        desconto = portugues.toString((portugues.toFloat(descontoInicial) / 100) * portugues.toFloat(Total()));
    else
        desconto = descontoInicial;

    QString valorFinal = portugues.toString(obterValorFinal(novaTaxa, desconto), 'f', 2);
    ui->Lbl_TotalTaxa->setText(valorFinal);
    ui->Lbl_Total->setText(valorFinal);
    ui->Ledit_Recebido->setText(valorFinal);
    ui->Lbl_Troco->setText("0");
}

void venda::on_CBox_FormaPagamento_activated(int index)
{
    QString taxaDebito  = QString::number(configDTO.taxaDebitoFinanceiro);
    QString taxaCredito = QString::number(configDTO.taxaCreditoFinanceiro);

    switch (index) {
    case 0: // dinheiro
        ui->label_2->show(); ui->label_3->show(); ui->Ledit_Recebido->show(); ui->Lbl_Troco->show();
        ui->lbl_taxa->hide(); ui->Ledit_Taxa->hide();
        ui->Ledit_Recebido->setText(Total());
        ui->Lbl_Troco->setText("0");
        ui->Ledit_Desconto->setText("0");
        ui->Ledit_Taxa->setText("0");
        ui->Lbl_TotalTaxa->setText(Total());
        ui->Lbl_Total->setText(Total());
        break;
    case 2: // crédito
        ui->label_2->hide(); ui->label_3->hide(); ui->Ledit_Recebido->hide(); ui->Lbl_Troco->hide();
        ui->lbl_taxa->show(); ui->Ledit_Taxa->show();
        ui->Ledit_Desconto->setText("0");
        ui->Ledit_Taxa->setText(taxaCredito);
        ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(taxaCredito, "0"), 'f', 2));
        ui->Lbl_Total->setText(ui->Lbl_TotalTaxa->text());
        break;
    case 3: // débito
        ui->label_2->hide(); ui->label_3->hide(); ui->Ledit_Recebido->hide(); ui->Lbl_Troco->hide();
        ui->lbl_taxa->show(); ui->Ledit_Taxa->show();
        ui->Ledit_Desconto->setText("0");
        ui->Ledit_Taxa->setText(taxaDebito);
        ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(taxaDebito, "0"), 'f', 2));
        ui->Lbl_Total->setText(ui->Lbl_TotalTaxa->text());
        break;
    default:
        ui->label_2->hide(); ui->label_3->hide(); ui->Ledit_Recebido->hide(); ui->Lbl_Troco->hide();
        ui->lbl_taxa->hide(); ui->Ledit_Taxa->hide();
        ui->Ledit_Desconto->setText("0");
        ui->Ledit_Taxa->setText("0");
        ui->Lbl_TotalTaxa->setText(Total());
        ui->Lbl_Total->setText(Total());
        break;
    }
}

void venda::on_Ledit_Recebido_textChanged(const QString &)
{
    float troco = portugues.toFloat(ui->Ledit_Recebido->text())
                  - portugues.toFloat(ui->Lbl_TotalTaxa->text());
    ui->Lbl_Troco->setText(portugues.toString(troco, 'f', 2));
}

void venda::on_Ledit_Taxa_textChanged(const QString &) { descontoTaxa(); }
void venda::on_Ledit_Desconto_textChanged(const QString &) { descontoTaxa(); }
void venda::on_CheckPorcentagem_stateChanged(int) { descontoTaxa(); }

void venda::on_CBox_ModeloEmit_currentIndexChanged(int index)
{
    if (index == 0) {
        ui->Ledit_NNF->setText(QString::number(
            notaServ.getProximoNNF(configDTO.tpAmbFiscal, ModeloNota::NFCe)));
        ui->RadioBtn_EmitNfApenas->setVisible(true);
        ui->RadioBtn_EmitNfTodos->setVisible(true);
        ui->Ledit_NNF->setVisible(true);
        ui->Lbl_NNF->setVisible(true);
    } else if (index == 1) {
        ui->Ledit_NNF->setText(QString::number(
            notaServ.getProximoNNF(configDTO.tpAmbFiscal, ModeloNota::NFe)));
        ui->RadioBtn_EmitNfApenas->setVisible(true);
        ui->RadioBtn_EmitNfTodos->setVisible(true);
        ui->Ledit_NNF->setVisible(true);
        ui->Lbl_NNF->setVisible(true);
    } else if (index == 2) {
        ui->RadioBtn_EmitNfApenas->setVisible(false);
        ui->RadioBtn_EmitNfTodos->setVisible(false);
        ui->Ledit_NNF->setVisible(false);
        ui->Lbl_NNF->setVisible(false);
    }
}

// ─── Finish sale ─────────────────────────────────────────────────────────────

void venda::terminarPagamento()
{
    QString troco          = ui->Lbl_Troco->text();
    QString recebido       = ui->Ledit_Recebido->text();
    QString forma          = ui->CBox_FormaPagamento->currentText();
    QString taxa           = ui->Ledit_Taxa->text();
    QString valor_final    = ui->Lbl_TotalTaxa->text();
    QString descontoInicial = ui->Ledit_Desconto->text();
    QString desconto;

    if (ui->CheckPorcentagem->isChecked())
        desconto = portugues.toString(
            (portugues.toFloat(descontoInicial) / 100) * portugues.toFloat(Total()));
    else
        desconto = descontoInicial;

    emitTodosNf = ui->RadioBtn_EmitNfTodos->isChecked();

    QString data = portugues.toString(ui->DateEdt_Venda->dateTime(), "dd-MM-yyyy hh:mm:ss");
    QDateTime dataIngles = portugues.toDateTime(data, "dd-MM-yyyy hh:mm:ss");

    VendasDTO newVenda;
    newVenda.clienteNome    = CLIENTE.nome;
    newVenda.dataHora       = dataIngles.toString("yyyy-MM-dd hh:mm:ss");
    newVenda.desconto       = portugues.toDouble(desconto);
    newVenda.formaPagamento = forma;
    newVenda.estaPago       = (forma != "Prazo");
    newVenda.idCliente      = idClienteAtual;
    newVenda.taxa           = portugues.toDouble(taxa);
    newVenda.total          = portugues.toDouble(Total());
    newVenda.troco          = portugues.toDouble(troco);
    newVenda.valorFinal     = portugues.toDouble(valor_final);
    newVenda.valorRecebido  = portugues.toDouble(recebido);

    if(idClienteAtual > 0)
        CLIENTE = cliServ.getClienteByID(idClienteAtual);
    ClienteDTO cli = CLIENTE;
    cli.cpf = ui->Ledit_CpfCnpjCliente->text().trimmed();

    QList<ProdutoVendidoDTO> produtos = obterProdutosSelecionados();

    auto result = vendaServ.inserirVendaRegraDeNegocio(newVenda, produtos);
    if (!result.ok) {
        QMessageBox::warning(this, "Erro", result.msg);
        return;
    }
    newVenda.id = result.idVendaInserida;

    if (ui->CheckImprimirCNF->isChecked())
        Vendas::imprimirReciboVenda(newVenda.id);

    if (configDTO.emitNfFiscal) {
        qlonglong nnf = ui->Ledit_NNF->text().toLongLong();
        int modeloIdx = ui->CBox_ModeloEmit->currentIndex();

        auto handleResultNf = [&](auto result1) {
            if (!result1.ok && result1.erro == FiscalEmitterErro::NCMInvalido) {
                auto resp = QMessageBox::question(this, "Atenção",
                    result1.msg + "\nDeseja continuar mesmo assim?",
                    QMessageBox::Yes | QMessageBox::No);
                if (resp == QMessageBox::No) {
                    auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                    if (!r1.ok) QMessageBox::warning(this, "Erro", r1.msg);
                    return false;
                }
            }
            if (!waitDialog) waitDialog = new WaitDialog(this);
            waitDialog->setMessage("Aguardando resposta do servidor...");
            waitDialog->show();
            waitDialog->allowClose();
            if (!result1.ok) {
                waitDialog->setMessage(result1.msg);
                auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                if (!r1.ok) QMessageBox::warning(this, "Erro", r1.msg);
                return false;
            }
            waitDialog->setMessage(result1.msg);
            QTimer::singleShot(1500, waitDialog, &WaitDialog::close);
            return true;
        };

        if (modeloIdx == 0) {
            auto r = fiscalServ.enviarNfcePadrao(newVenda, produtos, nnf, cli, emitTodosNf, false);
            if (!r.ok && r.erro == FiscalEmitterErro::NCMInvalido) {
                auto resp = QMessageBox::question(this, "Atenção",
                    r.msg + "\nDeseja continuar mesmo assim?", QMessageBox::Yes | QMessageBox::No);
                if (resp == QMessageBox::No) {
                    auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                    if (!r1.ok) QMessageBox::warning(this, "Erro", r1.msg);
                    return;
                }
                r = fiscalServ.enviarNfcePadrao(newVenda, produtos, nnf, cli, emitTodosNf, true);
            }
            if (!waitDialog) waitDialog = new WaitDialog(this);
            waitDialog->setMessage("Aguardando resposta do servidor...");
            waitDialog->show();
            waitDialog->allowClose();
            if (!r.ok) {
                waitDialog->setMessage(r.msg);
                auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                if (!r1.ok) QMessageBox::warning(this, "Erro", r1.msg);
                return;
            }
            waitDialog->setMessage(r.msg);
            QTimer::singleShot(1500, waitDialog, &WaitDialog::close);

        } else if (modeloIdx == 1) {
            auto r = fiscalServ.enviarNFePadrao(newVenda, produtos, nnf, cli, emitTodosNf, false);
            if (!r.ok && r.erro == FiscalEmitterErro::NCMInvalido) {
                auto resp = QMessageBox::question(this, "Atenção",
                    r.msg + "\nDeseja continuar mesmo assim?", QMessageBox::Yes | QMessageBox::No);
                if (resp == QMessageBox::No) {
                    auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                    if (!r1.ok) QMessageBox::warning(this, "Erro", r1.msg);
                    return;
                }
                r = fiscalServ.enviarNFePadrao(newVenda, produtos, nnf, cli, emitTodosNf, true);
            }
            if (!waitDialog) waitDialog = new WaitDialog(this);
            waitDialog->setMessage("Aguardando resposta do servidor...");
            waitDialog->show();
            waitDialog->allowClose();
            if (!r.ok) {
                waitDialog->setMessage(r.msg);
                auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                if (!r1.ok) QMessageBox::warning(this, "Erro", r1.msg);
                return;
            }
            waitDialog->setMessage(r.msg);
            QTimer::singleShot(1500, waitDialog, &WaitDialog::close);
        }
        // index 2 = Não Emitir NF — nada a fazer
    }

    descartarRascunho();
    emit vendaConcluida();
    this->close();
}

// ─── Products page ────────────────────────────────────────────────────────────

void venda::atualizarTotalProduto()
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

void venda::atualizarListaCliente()
{
    clientesComId = cliServ.listarClientesParaCompleter();
    QCompleter *completer = ui->Ledit_Cliente->completer();
    if (completer) {
        QStringListModel *model = qobject_cast<QStringListModel *>(completer->model());
        if (model) model->setStringList(clientesComId);
    }
}

venda::~venda() { delete ui; }

// ─── Rascunho ────────────────────────────────────────────────────────────────

void venda::salvarRascunho()
{
    RascunhoVendaSaveDTO dto;

    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        ProdutoVendidoDTO p;
        p.idProduto    = modeloSelecionados->item(row, 0)->text().toLongLong();
        p.quantidade   = portugues.toDouble(modeloSelecionados->item(row, 1)->text());
        p.descricao    = modeloSelecionados->item(row, 2)->text();
        p.precoVendido = portugues.toDouble(modeloSelecionados->item(row, 3)->text());
        dto.produtos.append(p);
    }

    // extrai ID direto do campo — idClienteAtual só é setado ao clicar "Próximo"
    auto [nomeIgnorado, idCli] = cliServ.extrairNomeId(ui->Ledit_Cliente->text());
    dto.idCliente           = idCli > 0 ? idCli : idClienteAtual;
    dto.cpfManual           = ui->Ledit_CpfCnpjCliente->text().trimmed();
    dto.dataHora            = ui->DateEdt_Venda->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    dto.formaPagamento      = ui->CBox_FormaPagamento->currentText();
    dto.desconto            = ui->Ledit_Desconto->text();
    dto.taxa                = ui->Ledit_Taxa->text();
    dto.recebido            = ui->Ledit_Recebido->text();
    dto.descontoPorcentagem = ui->CheckPorcentagem->isChecked();
    dto.modeloNf            = ui->CBox_ModeloEmit->currentIndex();
    dto.emitirTodos         = ui->RadioBtn_EmitNfTodos->isChecked();

    rascunhoServ.salvar(dto);
}

void venda::descartarRascunho()
{
    rascunhoServ.descartar();
}

void venda::verificarRascunho()
{
    if (!rascunhoServ.existe()) return;

    RascunhoVendaDTO rascunho = rascunhoServ.carregar();
    QList<ProdutoVendidoDTO> produtos = rascunhoServ.carregarProdutos(rascunho.produtosJson);
    if (produtos.isEmpty()) { descartarRascunho(); return; }

    auto resp = QMessageBox::question(this, "Rascunho de venda",
        QString("Existe um rascunho com %1 produto(s) não finalizado.\n"
                "Deseja continuar de onde parou?").arg(produtos.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (resp == QMessageBox::No) { descartarRascunho(); return; }

    // restaura produtos
    for (const ProdutoVendidoDTO &p : produtos) {
        QStandardItem *itemPreco = new QStandardItem();
        itemPreco->setData(p.precoVendido, Qt::EditRole);
        itemPreco->setText(portugues.toString(p.precoVendido, 'f', 2));

        QStandardItem *itemTotal = new QStandardItem();
        itemTotal->setData(p.quantidade * p.precoVendido, Qt::EditRole);
        itemTotal->setText(portugues.toString(p.quantidade * p.precoVendido, 'f', 2));

        modeloSelecionados->appendRow({
            new QStandardItem(QString::number(p.idProduto)),
            new QStandardItem(portugues.toString(p.quantidade, 'f', 2)),
            new QStandardItem(p.descricao),
            itemPreco,
            itemTotal
        });
    }

    // restaura cliente e data
    if (rascunho.idCliente > 0) {
        for (const QString &s : clientesComId) {
            if (s.contains(QString("(ID: %1)").arg(rascunho.idCliente))) {
                ui->Ledit_Cliente->setText(s);
                idClienteAtual = rascunho.idCliente;
                break;
            }
        }
    }

    if (!rascunho.dataHora.isEmpty())
        ui->DateEdt_Venda->setDateTime(QDateTime::fromString(rascunho.dataHora, "yyyy-MM-dd HH:mm:ss"));

    // pagamento e NF serão aplicados em configurarPaginaPagamento/configurarPaginaNF
    // para não serem sobrescritos pelos defaults dessas funções
    rascunhoPendente    = rascunho;
    temRascunhoPendente = true;

    ui->Lbl_Total->setText(Total());
}

qlonglong venda::validarCliente(bool mostrarMensagens)
{
    auto resultado = cliServ.validarClienteTexto(ui->Ledit_Cliente->text());
    if (!resultado.ok) {
        if (mostrarMensagens) {
            switch (resultado.erro) {
            case ClienteErro::CampoVazio:
                QMessageBox::warning(this, "Cliente", "Por favor informe o cliente.");
                ui->Ledit_Cliente->setFocus();
                break;
            case ClienteErro::InsercaoInvalida:
                QMessageBox::warning(this, "Cliente", "Cliente não encontrado.");
                ui->Ledit_Cliente->clear();
                ui->Ledit_Cliente->setFocus();
                break;
            case ClienteErro::QuebraDeRegra:
                QMessageBox::warning(this, "Cliente", "Nome não corresponde ao ID.");
                ui->Ledit_Cliente->selectAll();
                ui->Ledit_Cliente->setFocus();
                break;
            default:
                QMessageBox::warning(this, "Erro", resultado.msg);
            }
        }
        return -1;
    }
    if (!resultado.nomeCorrigido.isEmpty())
        ui->Ledit_Cliente->setText(resultado.nomeCorrigido);
    return resultado.clienteId;
}

void venda::on_Btn_SelecionarProduto_clicked()
{
    QItemSelectionModel *sel = ui->Tview_Produtos->selectionModel();
    QModelIndex idx = sel->selectedIndexes().first();
    QString idProduto   = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(idx.row(), 0)).toString();
    QString descProduto = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(idx.row(), 2)).toString();
    float   precoProduto = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(idx.row(), 3)).toFloat();

    QStandardItem *itemPreco = new QStandardItem();
    itemPreco->setData(precoProduto, Qt::EditRole);
    itemPreco->setText(portugues.toString(precoProduto, 'f', 2));

    QStandardItem *itemTotal = new QStandardItem();
    itemTotal->setData(precoProduto, Qt::EditRole);
    itemTotal->setText(portugues.toString(precoProduto, 'f', 2));

    modeloSelecionados->appendRow({new QStandardItem(idProduto), new QStandardItem("1"),
                                   new QStandardItem(descProduto), itemPreco, itemTotal});
    ui->Lbl_Total->setText(Total());
}

void venda::handleSelectionChange(const QItemSelection &, const QItemSelection &) {}

void venda::handleSelectionChangeProdutos(const QItemSelection &selected, const QItemSelection &)
{
    ui->Btn_SelecionarProduto->setEnabled(!selected.indexes().isEmpty());
}

void venda::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        irParaPagina(0);
        ui->Ledit_Pesquisa->setFocus();
        ui->Ledit_Pesquisa->selectAll();
    } else if (event->key() == Qt::Key_Escape) {
        ui->Btn_CancelarVenda->click();
    } else if (event->key() == Qt::Key_F4) {
        ui->Tview_Produtos->setFocus();
    } else if (event->key() == Qt::Key_F2) {
        irParaPagina(1);
        ui->Ledit_Cliente->setFocus();
        ui->Ledit_Cliente->selectAll();
    } else if (event->key() == Qt::Key_F3) {
        irParaPagina(1);
        ui->DateEdt_Venda->setFocus();
    } else if (event->key() == Qt::Key_F9) {
        ui->Tview_ProdutosSelecionados->setFocus();
    } else if (ui->Tview_Produtos->hasFocus() &&
               (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        ui->Btn_SelecionarProduto->click();
    } else if (ui->Tview_ProdutosSelecionados->hasFocus() && event->key() == Qt::Key_Delete) {
        deletarProd();
    }
    QWidget::keyPressEvent(event);
}

void venda::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    ui->Ledit_Cliente->setReadOnly(false);
}

void venda::on_Btn_Pesquisa_clicked()
{
    prodServ.pesquisar(ui->Ledit_Pesquisa->text(), modeloProdutos);
    if (!modeloProdutos)
        QMessageBox::warning(this, "Erro", "Erro ao realizar a pesquisa.");
}

QList<ProdutoVendidoDTO> venda::obterProdutosSelecionados()
{
    QList<ProdutoVendidoDTO> lista;
    QLocale brasil(QLocale::Portuguese, QLocale::Brazil);
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        ProdutoVendidoDTO prod;
        prod.idProduto   = modeloSelecionados->data(modeloSelecionados->index(row, 0)).toLongLong();
        prod.quantidade  = brasil.toDouble(modeloSelecionados->data(modeloSelecionados->index(row, 1)).toString());
        prod.descricao   = modeloSelecionados->data(modeloSelecionados->index(row, 2)).toString();
        prod.precoVendido = brasil.toDouble(modeloSelecionados->data(modeloSelecionados->index(row, 3)).toString());
        lista.append(prod);
    }
    return lista;
}

QString venda::Total()
{
    double totalValue = 0.0;
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        float quantidade = portugues.toFloat(
            modeloSelecionados->data(modeloSelecionados->index(row, 1)).toString());
        double preco = portugues.toDouble(
            modeloSelecionados->data(modeloSelecionados->index(row, 3)).toString());
        totalValue += quantidade * preco;
    }
    return portugues.toString(totalValue, 'f', 2);
}

void venda::on_Ledit_Pesquisa_textChanged(const QString &)
{
    ui->Btn_Pesquisa->click();
}

void venda::on_Tview_ProdutosSelecionados_customContextMenuRequested(const QPoint &pos)
{
    if (!ui->Tview_ProdutosSelecionados->currentIndex().isValid()) return;
    QMenu menu(this);
    menu.addAction(actionMenuDeletarProd);
    menu.exec(ui->Tview_ProdutosSelecionados->viewport()->mapToGlobal(pos));
}

void venda::deletarProd()
{
    modeloSelecionados->removeRow(ui->Tview_ProdutosSelecionados->currentIndex().row());
    ui->Lbl_Total->setText(Total());
}

void venda::on_Ledit_Pesquisa_returnPressed()
{
    QString barras = ui->Ledit_Pesquisa->text();
    if (!prodServ.codigoBarrasExiste(barras)) {
        QMessageBox::warning(this, "Erro", "Esse código de barras não foi registrado ainda.");
        return;
    }
    ProdutoDTO prod = prodServ.getProdutoPeloCodBarras(barras);
    modeloSelecionados->appendRow({
        new QStandardItem(QString::number(prod.id)),
        new QStandardItem("1"),
        new QStandardItem(prod.descricao),
        new QStandardItem(portugues.toString(prod.preco))
    });
    atualizarTotalProduto();
    ui->Ledit_Pesquisa->clear();
    ui->Lbl_Total->setText(Total());
}

void venda::on_Btn_CancelarVenda_clicked()
{
    if (modeloSelecionados->rowCount() > 0) {
        auto resp = QMessageBox::question(this, "Cancelar Venda",
            "Deseja salvar um rascunho para continuar esta venda depois?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (resp == QMessageBox::Cancel) return;
        if (resp == QMessageBox::No) descartarRascunho();
    }
    this->close();
}

void venda::selecionarClienteNovo()
{
    atualizarListaCliente();
    if (!clientesComId.isEmpty()) {
        ui->Ledit_Cliente->setText(clientesComId.last());
        int posFinalNome = clientesComId.last().indexOf(" (ID:");
        if (posFinalNome != -1)
            ui->Ledit_Cliente->setSelection(0, posFinalNome);
    }
}

void venda::on_Btn_NovoCliente_clicked()
{
    InserirCliente *inserirCliente = new InserirCliente;
    inserirCliente->setWindowModality(Qt::ApplicationModal);
    connect(inserirCliente, &InserirCliente::clienteInserido, this, &venda::selecionarClienteNovo);
    inserirCliente->show();
}

QString venda::getIdProdSelected()
{
    QItemSelectionModel *sel = ui->Tview_Produtos->selectionModel();
    QModelIndexList selectedIndexes = sel->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        int row = selectedIndexes.first().row();
        return ui->Tview_Produtos->model()->data(
            ui->Tview_Produtos->model()->index(row, 0)).toString();
    }
    return {};
}

void venda::verProd()
{
    QString id = getIdProdSelected();
    InfoJanelaProd *janelaProd = new InfoJanelaProd(this, id);
    janelaProd->show();
}
