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
    QItemSelectionModel *selectionModelProdutos = ui->Tview_Produtos->selectionModel();
    connect(selectionModelProdutos, &QItemSelectionModel::selectionChanged, this,
            &venda::handleSelectionChangeProdutos);

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

        // Isso depende do formato que você está usando ("Nome (ID: 123)")
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

    fiscalValues = Configuracao::get_All_Fiscal_Values();
    QString tipoAmb = fiscalValues.value("tp_amb");
    QString emitirNf = fiscalValues.value("emit_nf");

    if (tipoAmb == "1" && emitirNf == "1") {
        ui->Lbl_TpAmb->setText("🟢 Amb: Produção");
        ui->Lbl_TpAmb->setStyleSheet("color: white; background-color: green; font-weight: bold; padding: 4px; border-radius: 5px;");
    } else if(emitirNf == "1"){
        ui->Lbl_TpAmb->setText("🟠 Amb: Homologação");
        ui->Lbl_TpAmb->setStyleSheet("color: white; background-color: orange; font-weight: bold; padding: 4px; border-radius: 5px;");
    }


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
            QString::number(totalproduto, 'f', 2) // 2 casas decimais
            );
    }
}
void venda::atualizarListaCliente(){
    clientesComId.clear(); // Limpa a lista antes de recarregar

    if (!db.open()) {
        qDebug() << "Erro ao conectar ao banco de dados para autocompletar nomes.";
    } else {
        // Consultar nomes e IDs da tabela "clientes"
        QSqlQuery query("SELECT id, nome FROM clientes");

        while (query.next()) {
            int id = query.value(0).toInt();
            QString nome = query.value(1).toString();
            // Formatar como "Nome (ID: 123)"
            clientesComId << QString("%1 (ID: %2)").arg(nome).arg(id);
        }
        db.close();
    }

    // Atualizar o completer
    QCompleter *completer = ui->Ledit_Cliente->completer();
    if (completer) {
        QStringListModel *model = qobject_cast<QStringListModel*>(completer->model());
        if (model) {
            model->setStringList(clientesComId);
        }
    }
}


venda::~venda()
{
    delete ui;
}
int venda::validarCliente(bool mostrarMensagens) {
    QString texto = ui->Ledit_Cliente->text().trimmed();

   // Se o campo estiver vazio
    if (texto.isEmpty()) {
        if (mostrarMensagens) {
            QMessageBox::warning(this, "Cliente não informado", "Por favor, informe o cliente!");
            ui->Ledit_Cliente->setFocus();
        }
        return -1; // Código de erro para campo vazio
    }

    // Extrai nome e ID do texto digitado
    auto [nome, id] = extrairNomeId(texto);

    // Verifica formato válido
    if (id == -1) {
        // Tenta encontrar o cliente mais próximo no banco de dados
        if (!db.open()) {
            if (mostrarMensagens) {
                qDebug() << "Erro ao abrir banco de dados";
                QMessageBox::warning(this, "Erro", "Não foi possível validar o cliente!");
            }
            return -2; // Código de erro para falha no banco de dados
        }

        QSqlQuery query;
        query.prepare("SELECT id, nome FROM clientes WHERE nome LIKE :nome ORDER BY LENGTH(nome) ASC LIMIT 1");
        query.bindValue(":nome", "%" + texto + "%");

        if (query.exec() && query.next()) {
            int foundId = query.value(0).toInt();
            QString foundName = query.value(1).toString();
            ui->Ledit_Cliente->setText(QString("%1 (ID: %2)").arg(foundName).arg(foundId));
            db.close();
            return foundId; // Retorna o ID encontrado
        } else {
            db.close();
            if (mostrarMensagens) {
                QMessageBox::warning(this, "Cliente não encontrado",
                                     "Nenhum cliente correspondente foi encontrado.\n"
                                     "Digite no formato: Nome (ID: 123) ou selecione uma sugestão.");
                ui->Ledit_Cliente->clear();
                ui->Ledit_Cliente->setFocus();
            }
            return -3; // Código de erro para cliente não encontrado
        }
    }

    // Verifica correspondência entre nome e ID
    if (!verificarNomeIdCliente(nome, id)) {
        if (mostrarMensagens) {
            QMessageBox::warning(this, "Dados inválidos",
                                 "O nome não corresponde ao ID informado!\n"
                                 "Por favor, corrija ou selecione uma sugestão válida.");
            ui->Ledit_Cliente->selectAll();
            ui->Ledit_Cliente->setFocus();
        }
        return -4; // Código de erro para nome e ID não correspondentes
    }

    return id; // Retorna o ID válido
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

    modeloSelecionados->appendRow({new QStandardItem(idProduto), itemQuantidade, new QStandardItem(descProduto), itemPreco});
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
    QString normalizadoPesquisa = MainWindow::normalizeText(inputText);

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


bool venda::verificarNomeIdCliente(const QString &nome, int id) {
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

QPair<QString, int> venda::extrairNomeId(const QString &texto) {
        QRegularExpression regex("^(.*?)\\s*\\(ID:\\s*(\\d+)\\)$");
        QRegularExpressionMatch match = regex.match(texto);

        if (match.hasMatch()) {
            return qMakePair(match.captured(1).trimmed(), match.captured(2).toInt());
        }
        return qMakePair(QString(), -1); // Retorno inválido
}




void venda::on_Btn_Aceitar_clicked()
{
    int idCliente = validarCliente(true);
    if (idCliente < 0) { // Se retornou algum código de erro
        return;
    }

    auto [nome, id] = extrairNomeId(ui->Ledit_Cliente->text());


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

    //QString cliente = ui->Ledit_Cliente->text();
    QString data =  portugues.toString(ui->DateEdt_Venda->dateTime(), "dd-MM-yyyy hh:mm:ss");
    pagamentoVenda *pagamento = new pagamentoVenda(rowDataList, Total(), nome, data, idCliente);
    pagamento->setWindowModality(Qt::ApplicationModal);
    connect(pagamento, &pagamento::pagamentoConcluido, this, &venda::vendaConcluida);
    connect(pagamento, &pagamento::pagamentoConcluido, this, &venda::close);




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

        // Opcional: selecionar apenas o nome (se quiser destacar parte do texto)
        // Isso depende do formato que você está usando ("Nome (ID: 123)")
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

