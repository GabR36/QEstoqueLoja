#include "janelaorcamento.h"
#include "ui_janelaorcamento.h"
#include <QDebug>
#include <QMessageBox>
#include <QCompleter>
#include <QStringListModel>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include "delegateprecovalidate.h"
#include "delegatelockcol.h"
#include "delegatequant.h"
#include "subclass/produtotableview.h"
#include "inserircliente.h"
#include "../services/Produto_service.h"
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

bool JanelaOrcamento::verificarNomeIdCliente(const QString &nome, int id)
{
    if (!db.open()) {
        qDebug() << "Erro ao conectar ao banco de dados";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT nome FROM clientes WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erro na consulta:";
        db.close();
        return false;
    }

    if (query.next()) {
        QString nomeNoBanco = query.value(0).toString();
        db.close();
        return nomeNoBanco.compare(nome, Qt::CaseInsensitive) == 0;
    }

    db.close();
    return false;
}

QPair<QString, int> JanelaOrcamento::extrairNomeId(const QString &texto)
{
    QRegularExpression regex("^(.*?)\\s*\\(ID:\\s*(\\d+)\\)$");
    QRegularExpressionMatch match = regex.match(texto);

    if (match.hasMatch()) {
        return qMakePair(match.captured(1).trimmed(), match.captured(2).toInt());
    }
    return qMakePair(QString(), -1);
}

int JanelaOrcamento::validarCliente(bool mostrarMensagens)
{
    QString texto = ui->Ledit_Cliente->text().trimmed();

    if (texto.isEmpty()) {
        if (mostrarMensagens) {
            QMessageBox::warning(this, "Cliente não informado", "Por favor, informe o cliente!");
            ui->Ledit_Cliente->setFocus();
        }
        return -1;
    }

    auto [nome, id] = extrairNomeId(texto);

    if (id == -1) {
        if (!db.open()) {
            if (mostrarMensagens) {
                QMessageBox::warning(this, "Erro", "Não foi possível validar o cliente!");
            }
            return -2;
        }

        QSqlQuery query;
        query.prepare("SELECT id, nome FROM clientes WHERE nome LIKE :nome ORDER BY LENGTH(nome) ASC LIMIT 1");
        query.bindValue(":nome", "%" + texto + "%");

        if (query.exec() && query.next()) {
            int foundId = query.value(0).toInt();
            QString foundName = query.value(1).toString();
            ui->Ledit_Cliente->setText(QString("%1 (ID: %2)").arg(foundName).arg(foundId));
            db.close();
            return foundId;
        } else {
            db.close();
            if (mostrarMensagens) {
                QMessageBox::warning(this, "Cliente não encontrado",
                                     "Nenhum cliente correspondente foi encontrado.\n"
                                     "Digite no formato: Nome (ID: 123) ou selecione uma sugestão.");
                ui->Ledit_Cliente->clear();
                ui->Ledit_Cliente->setFocus();
            }
            return -3;
        }
    }

    if (!verificarNomeIdCliente(nome, id)) {
        if (mostrarMensagens) {
            QMessageBox::warning(this, "Dados inválidos",
                                 "O nome não corresponde ao ID informado!\n"
                                 "Por favor, corrija ou selecione uma sugestão válida.");
            ui->Ledit_Cliente->selectAll();
            ui->Ledit_Cliente->setFocus();
        }
        return -4;
    }

    return id;
}

void JanelaOrcamento::atualizarListaCliente()
{
    clientesComId.clear();

    if (!db.open()) {
        qDebug() << "Erro ao conectar ao banco de dados para autocompletar nomes.";
    } else {
        QSqlQuery query("SELECT id, nome FROM clientes");

        while (query.next()) {
            int id = query.value(0).toInt();
            QString nome = query.value(1).toString();
            clientesComId << QString("%1 (ID: %2)").arg(nome).arg(id);
        }
    }

    QCompleter *completer = ui->Ledit_Cliente->completer();
    if (completer) {
        QStringListModel *model = qobject_cast<QStringListModel*>(completer->model());
        if (model) {
            model->setStringList(clientesComId);
        }
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
        int posInicioNome = 0;
        int posFinalNome = primeiroCliente.indexOf(" (ID:");

        if (posFinalNome != -1) {
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
        double totalproduto = 0.0;

        float quantidade = portugues.toFloat(
            modeloSelecionados->data(modeloSelecionados->index(row, 1)).toString());
        double preco = portugues.toDouble(
            modeloSelecionados->data(modeloSelecionados->index(row, 3)).toString());

        totalproduto = quantidade * preco;

        modeloSelecionados->setData(
            modeloSelecionados->index(row, 4),
            portugues.toString(totalproduto, 'f', 2));
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
    QVariant idVariant = modelo->data(modelo->index(selectedIndex.row(), 0));
    QVariant descVariant = modelo->data(modelo->index(selectedIndex.row(), 2));
    QVariant precoVariant = modelo->data(modelo->index(selectedIndex.row(), 3));

    QString idProduto = idVariant.toString();
    QString descProduto = descVariant.toString();

    float precoProduto = precoVariant.toFloat();
    QStandardItem *itemPreco = new QStandardItem();
    itemPreco->setData(precoProduto, Qt::EditRole);
    itemPreco->setText(portugues.toString(precoProduto, 'f', 2));

    QStandardItem *itemQuantidade = new QStandardItem("1");

    QStandardItem *itemTotal = new QStandardItem();
    itemTotal->setData(precoProduto, Qt::EditRole);
    itemTotal->setText(portugues.toString(precoProduto, 'f', 2));

    modeloSelecionados->appendRow({
        new QStandardItem(idProduto),
        itemQuantidade,
        new QStandardItem(descProduto),
        itemPreco,
        itemTotal
    });

    ui->Lbl_TotalGeral->setText(totalGeral());
}

void JanelaOrcamento::on_Ledit_PesquisaProduto_textChanged(const QString &arg1)
{
    QString inputText = ui->Ledit_PesquisaProduto->text();
    QString normalizadoPesquisa = Produto_Service::normalizeText(inputText);

    QStringList palavras = normalizadoPesquisa.split(" ", Qt::SkipEmptyParts);

    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados. Pesquisar orçamento.";
        return;
    }

    QString sql = "SELECT * FROM produtos WHERE ";
    QStringList conditions;
    if (palavras.length() > 1) {
        for (const QString &palavra : palavras) {
            conditions << QString("descricao LIKE '%%1%'").arg(palavra);
        }
        sql += conditions.join(" AND ");
    } else {
        sql += "descricao LIKE '%" + normalizadoPesquisa + "%'  OR codigo_barras LIKE '%" + normalizadoPesquisa + "%'";
    }
    sql += " ORDER BY id DESC";

    ProdutoTableView *ptv = qobject_cast<ProdutoTableView*>(ui->Tview_ProdutosOrcamento);
    QSqlQueryModel *modelo = ptv->getModel();

    modelo->setQuery(sql, db);
    ui->Tview_ProdutosOrcamento->setModel(modelo);

    db.close();
}

void JanelaOrcamento::on_Btn_Terminar_clicked()
{
    int idCliente = validarCliente(true);
    if (idCliente < 0) {
        return;
    }

    auto [nome, id] = extrairNomeId(ui->Ledit_Cliente->text());

    QList<QList<QVariant>> rowDataList;
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        QList<QVariant> rowData;
        for (int col = 0; col < modeloSelecionados->columnCount(); col++) {
            QModelIndex index = modeloSelecionados->index(row, col);
            rowData.append(modeloSelecionados->data(index));
        }
        rowDataList.append(rowData);
    }

    QString data = portugues.toString(QDateTime::currentDateTime(), "dd-MM-yyyy hh:mm:ss");
    if (!db.open()) {
        qDebug() << "bd nao abriu botao ver orcamento";
    }

    QMap<QString, QString> dadosEmpresa;

    QSqlQuery query;
    query.exec("SELECT key, value FROM config WHERE key IN ("
               "'nome_empresa', "
               "'endereco_empresa', "
               "'telefone_empresa', "
               "'cnpj_empresa', "
               "'email_empresa', "
               "'cidade_empresa', "
               "'estado_empresa', "
               "'caminho_logo_empresa')");

    while (query.next()) {
        QString key = query.value(0).toString();
        QString value = query.value(1).toString();
        dadosEmpresa[key] = value;
    }

    QString nomeEmpresa = dadosEmpresa.value("nome_empresa", "");
    QString caminhoLogo = dadosEmpresa.value("caminho_logo_empresa", "");
    QString endereco_empresa = dadosEmpresa.value("endereco_empresa", "");
    QString cnpj_empresa = dadosEmpresa.value("cnpj_empresa", "");
    QString email_empresa = dadosEmpresa.value("email_empresa", "");
    QString cidade_empresa = dadosEmpresa.value("cidade_empresa", "");
    QString estado_empresa = dadosEmpresa.value("estado_empresa", "");
    QString telefone_empresa = dadosEmpresa.value("telefone_empresa", "");

    query.prepare("SELECT email, telefone, endereco, cpf FROM clientes where id = :id");
    query.bindValue(":id", idCliente);
    QString emailCliente, telefoneCliente, enderecoCliente, cpfCliente;
    query.exec();
    while (query.next()) {
        emailCliente = query.value(0).toString();
        telefoneCliente = query.value(1).toString();
        enderecoCliente = query.value(2).toString();
        cpfCliente = query.value(3).toString();
    }
    db.close();

    QString observacao = ui->Tedit_Obs->toPlainText();

    QtRPT *report = new QtRPT(nullptr);

    QStringList dataLocations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    QString reportPath;

    for (const QString &basePath : dataLocations) {
        QString candidate = basePath + "/QEstoqueLoja/reports/orcamentoReport.xml";
        if (QFileInfo::exists(candidate)) {
            reportPath = candidate;
            break;
        }
    }
    report->loadReport(reportPath);

    connect(report, &QtRPT::setDSInfo, [&](DataSetInfo &dsinfo) {
        dsinfo.recordCount = rowDataList.size();
    });
    connect(report, &QtRPT::setValue, [&](const int recno, const QString paramname, QVariant &paramvalue, const int reportpage) {
        Q_UNUSED(reportpage);

        if (paramname == "nomeEmpresa") {
            paramvalue = nomeEmpresa;
        } else if (paramname == "endereco") {
            paramvalue = endereco_empresa;
        } else if (paramname == "cidade") {
            paramvalue = cidade_empresa;
        } else if (paramname == "estado") {
            paramvalue = estado_empresa;
        } else if (paramname == "email") {
            paramvalue = email_empresa;
        } else if (paramname == "cnpj") {
            paramvalue = cnpj_empresa;
        } else if (paramname == "telefone") {
            paramvalue = telefone_empresa;
        } else if (paramname == "obs") {
            paramvalue = observacao;
        } else if (paramname == "total_geral") {
            paramvalue = totalGeral();
        } else if (paramname == "nome_cliente") {
            paramvalue = nome;
        } else if (paramname == "endereco_cliente") {
            paramvalue = enderecoCliente;
        } else if (paramname == "cpf_cliente") {
            paramvalue = cpfCliente;
        } else if (paramname == "email_cliente") {
            paramvalue = emailCliente;
        } else if (paramname == "telefone_cliente") {
            paramvalue = telefoneCliente;
        } else if (paramname == "data") {
            paramvalue = data;
        }

        if (recno < rowDataList.size()) {
            auto rowData = rowDataList.at(recno);

            if (paramname == "id_produto") {
                paramvalue = rowData.at(0);
            } else if (paramname == "nome_produto") {
                paramvalue = rowData.at(2);
            } else if (paramname == "quantidade") {
                paramvalue = rowData.at(1);
            } else if (paramname == "preco_unitario") {
                paramvalue = rowData.at(3);
            } else if (paramname == "subtotal") {
                paramvalue = rowData.at(4);
            }
        }
    });
    connect(report, &QtRPT::setValueImage, [&](const int recno, const QString paramname, QImage &paramvalue, const int reportpage) {
        Q_UNUSED(reportpage);
        Q_UNUSED(recno);

        if (paramname == "imgLogo") {
            QString caminhoCompleto = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                      "/imagens/" + QFileInfo(caminhoLogo).fileName();

            if (!QFile::exists(caminhoCompleto)) {
                qDebug() << "Erro: Arquivo não encontrado:" << caminhoCompleto;
                return;
            }

            QImage img(caminhoCompleto);
            if (img.isNull()) {
                qDebug() << "Erro ao carregar imagem:" << caminhoCompleto;
            } else {
                paramvalue = img;
            }
        }
    });
    report->printExec();
}

void JanelaOrcamento::selecionarClienteNovo()
{
    atualizarListaCliente();
    if (!clientesComId.isEmpty()) {
        ui->Ledit_Cliente->setText(clientesComId.last());

        QString ultimoCliente = clientesComId.last();
        int posInicioNome = 0;
        int posFinalNome = ultimoCliente.indexOf(" (ID:");

        if (posFinalNome != -1) {
            ui->Ledit_Cliente->setSelection(posInicioNome, posFinalNome);
        }
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
