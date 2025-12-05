#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>
#include "customdelegate.h"
#include "alterarproduto.h"
#include "QItemSelectionModel"
#include <qsqltablemodel.h>
#include <QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include "vendas.h"
#include <QDoubleValidator>
#include "relatorios.h"
#include "venda.h"
#include <QModelIndex>
#include <QMenu>
#include <QFontDatabase>
#include <zint.h>
#include "delegateprecof2.h"
#include "util/pdfexporter.h"
#include "clientes.h"
#include "pagamentovenda.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QSql>
#include  "inserirproduto.h"
#include "subclass/leditdialog.h"
#include "infojanelaprod.h"
#include <QStandardPaths>
#include "util/helppage.h"
#include "schemamanager.h"
#include "util/consultacnpjmanager.h"
#include "entradas.h"
#include "util/manifestadordfe.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SchemaManager *schemaManager = new SchemaManager(this, 7);
    //config versao 6
    connect(schemaManager, &SchemaManager::dbVersao6, this,
            &MainWindow::atualizarConfigAcbr);
    schemaManager->update();

    db = QSqlDatabase::database();

    // configuracao do modelo e view produtos
    ui->Tview_Produtos->setModel(model);
    model->setQuery("SELECT * FROM produtos ORDER BY id DESC");
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Quantidade"));
    model->setHeaderData(2, Qt::Horizontal, tr("Descrição"));
    model->setHeaderData(3, Qt::Horizontal, tr("Preço"));
    model->setHeaderData(4, Qt::Horizontal, tr("Código de Barras"));
    model->setHeaderData(5, Qt::Horizontal, tr("NF"));

    financeiroValues = Configuracao::get_All_Financeiro_Values();
    empresaValues = Configuracao::get_All_Empresa_Values();
    fiscalValues = Configuracao::get_All_Fiscal_Values();

    //pega o ponteiro do singleton para usar a lib acbr
    // acbr = AcbrManager::instance()->nfe();

    // mostrar na tabela da aplicaçao a tabela do banco de dados.
    ui->Tview_Produtos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    ui->Ledit_Pesquisa->installEventFilter(this);
    atualizarTableview();
    //
    // Selecionar a primeira linha da tabela
    QModelIndex firstIndex = model->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);

    // ajustar tamanho colunas
    // coluna descricao
    ui->Tview_Produtos->setColumnWidth(2, 750);
    //preco
    ui->Tview_Produtos->setColumnWidth(3, 90);

    // coluna quantidade
    ui->Tview_Produtos->setColumnWidth(1, 85);
    ui->Tview_Produtos->setColumnWidth(4,110);

    // ações para menu de contexto tabela produtos
    actionMenuAlterarProd = new QAction(this);
    actionMenuDeletarProd = new QAction(this);
    actionSetLocalProd = new QAction(this);
    actionVerProduto = new QAction(this);
    actionSetLocalProd->setText("Adicionar Local Produto");
    actionVerProduto->setText("Ver Produto");
    connect(actionSetLocalProd,SIGNAL(triggered(bool)),this,SLOT(setLocalProd()));
    connect(actionVerProduto, SIGNAL(triggered(bool)),this,SLOT(verProd()));

    actionMenuDeletarProd->setText("Deletar Produto");
    actionMenuDeletarProd->setIcon(iconDelete);
    connect(actionMenuDeletarProd,SIGNAL(triggered(bool)),this,SLOT(on_Btn_Delete_clicked()));


    actionMenuAlterarProd->setText("Alterar Produto");
    actionMenuAlterarProd->setIcon(iconAlterarProduto);
    connect(actionMenuAlterarProd,SIGNAL(triggered(bool)),this,SLOT(on_Btn_Alterar_clicked()));

    actionMenuPrintBarCode1 = new QAction(this);
    actionMenuPrintBarCode1->setText("1 Etiqueta");
    connect(actionMenuPrintBarCode1,SIGNAL(triggered(bool)),this, SLOT(imprimirEtiqueta1()));

    actionMenuPrintBarCode3 = new QAction(this);
    actionMenuPrintBarCode3->setText("3 Etiquetas");
    connect(actionMenuPrintBarCode3,SIGNAL(triggered(bool)),this, SLOT(imprimirEtiqueta3()));
    // -- delegates --
    DelegatePrecoF2 *delegatePreco = new DelegatePrecoF2(this);
    ui->Tview_Produtos->setItemDelegateForColumn(3,delegatePreco);
    CustomDelegate *delegateVermelho = new CustomDelegate(this);
    ui->Tview_Produtos->setItemDelegateForColumn(1,delegateVermelho);

    setarIconesJanela();

    connect(ui->Tview_Produtos, &QTableView::doubleClicked,
            this, &MainWindow::verProd);

    // ManifestadorDFe *manifestdfe = new ManifestadorDFe();
    // if(manifestdfe->possoConsultar()){
    //     manifestdfe->consultarEManifestar();

    // }else{
    //     qDebug() << "Nao consultado DFE";

    // }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setarIconesJanela(){
    iconAlterarProduto.addFile(":/QEstoqueLOja/story-editor.svg");
    iconAddProduto.addFile(":/QEstoqueLOja/add-product.svg");
    iconBtnVenda.addFile(":/QEstoqueLOja/amarok-cart-view.svg");
    iconDelete.addFile(":/QEstoqueLOja/amarok-cart-remove.svg");
    iconPesquisa.addFile(":/QEstoqueLOja/edit-find.svg");
    iconBtnRelatorios.addFile(":/QEstoqueLOja/view-financial-account-investment-security.svg");
    iconImpressora.addFile(":/QEstoqueLOja/document-print.svg");
    iconClientes.addFile(":/QEstoqueLOja/user-others.svg");


    ui->Btn_AddProd->setIcon(iconAddProduto);
    ui->Btn_Venda->setIcon(iconBtnVenda);
    ui->Btn_Alterar->setIcon(iconAlterarProduto);
    ui->Btn_Delete->setIcon(iconDelete);
    ui->Btn_Pesquisa->setIcon(iconPesquisa);
    ui->Btn_Relatorios->setIcon(iconBtnRelatorios);
    ui->Btn_Clientes->setIcon(iconClientes);
}
void MainWindow::atualizarTableview(){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. atualizarTableView";
    }
    model->setQuery("SELECT * FROM produtos ORDER BY id DESC");
    db.close();
}


void MainWindow::on_Btn_Delete_clicked()
{
    if(ui->Tview_Produtos->selectionModel()->isSelected(ui->Tview_Produtos->currentIndex())){
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QString productId = idVariant.toString();
    QString productDesc = descVariant.toString();

    // Cria uma mensagem de confirmação
    QMessageBox::StandardButton resposta;
    resposta = QMessageBox::question(
        nullptr,
        "Confirmação",
        "Tem certeza que deseja excluir o produto:\n\n"
        "id: " + productId + "\n"
        "Descrição: " + productDesc,
        QMessageBox::Yes | QMessageBox::No
    );
    // Verifica a resposta do usuário
    if (resposta == QMessageBox::Yes) {

        // remover registro do banco de dados
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. botao deletar.";
        }
        QSqlQuery query;

        query.prepare("DELETE FROM produtos WHERE id = :valor1");
        query.bindValue(":valor1", productId);
        if (query.exec()) {
            qDebug() << "Delete bem-sucedido!";
        } else {
            qDebug() << "Erro no Delete: ";
        }
        atualizarTableview();
        db.close();
    }
    else {
        // O usuário escolheu não deletar o produto
        qDebug() << "A exclusão do produto foi cancelada.";
    }
    }else{
        QMessageBox::warning(this,"Erro","Selecione um produto antes de tentar deletar!");
    }
}
QString MainWindow::normalizeText(const QString &text) {
    QString normalized = text.normalized(QString::NormalizationForm_D);
    QString result;
    for (const QChar &c : normalized) {
        if (!c.isMark()) {
            QChar replacement;
            switch (c.unicode()) {
            case ';':
            case '\'':
            case '\"':
                // Remover os caracteres ; ' "
                continue;
            case '<':
                replacement = '(';
                break;
            case '>':
                replacement = ')';
                break;
            case '&':
                replacement = 'e';
                break;
            default:
                result.append(c.toUpper());
                continue;
            }
            result.append(replacement);
        }
    }
    return result;



}

void MainWindow::on_Btn_Pesquisa_clicked()
{
    QString inputText = ui->Ledit_Pesquisa->text();
    QString normalizadoPesquisa = normalizeText(inputText);

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
    model->setQuery(sql, db);
    if (model->lastError().isValid()) {
        qDebug() << "Erro ao executar consulta:" << model->lastError().text();
    }


    db.close();
}




void MainWindow::on_Btn_Alterar_clicked()
{


    if(ui->Tview_Produtos->selectionModel()->isSelected(ui->Tview_Produtos->currentIndex())){
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QVariant quantVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 1));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));
    QVariant barrasVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 4));
    QVariant nfVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 5));
    QVariant uCom = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 6));
    QVariant precoForn = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 7));
    QVariant porcentLucro = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 8));
    QVariant ncm = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 9));
    QVariant cest = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 10));
    QVariant aliquotaImp = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 11));
    QVariant csosn = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 12));
    QVariant pis = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 13));


    QString productId = idVariant.toString();
    QString productQuant = portugues.toString(quantVariant.toFloat());
    QString productDesc = descVariant.toString();
    QString productPreco = portugues.toString(precoVariant.toFloat());
    QString productBarras = barrasVariant.toString();
    bool productNf = nfVariant.toBool();
    QString productUCom = uCom.toString();
    QString produtoPrecoForn;
    if(precoForn.toString().trimmed().isEmpty()){
        produtoPrecoForn = "";
    }else{
        produtoPrecoForn = portugues.toString(precoForn.toFloat());
    }
    QString productPorcentLucro = portugues.toString(porcentLucro.toFloat());
    QString productNCM = ncm.toString();
    QString productCEST = cest.toString();
    QString productAliquotaImp = portugues.toString(aliquotaImp.toFloat());
    QString productPis = pis.toString();
    QString productCsosn = csosn.toString();

    qDebug() << productId;
    qDebug() << productPreco;
    // criar janela
    AlterarProduto *alterar = new AlterarProduto;
    alterar->janelaPrincipal = this;
    alterar->idAlt = productId;
    alterar->TrazerInfo(productDesc, productQuant, productPreco, productBarras, productNf, productUCom,
    produtoPrecoForn, productPorcentLucro, productNCM, productCEST, productAliquotaImp, productCsosn, productPis);
    alterar->setWindowModality(Qt::ApplicationModal);
    alterar->show();
    connect(alterar, &AlterarProduto::produtoAlterado, this,
            &MainWindow::on_Btn_Pesquisa_clicked);
    }else{
        QMessageBox::warning(this,"Erro","Selecione um produto antes de alterar!");
    }

}


void MainWindow::on_Btn_Venda_clicked()
{
    Vendas *vendas = new Vendas;
    //vendas->setWindowModality(Qt::ApplicationModal);
    connect(vendas, &Vendas::vendaConcluidaVendas, this, &MainWindow::atualizarTableview);

    vendas->show();


}



void MainWindow::on_Btn_Relatorios_clicked()
{
    relatorios *relatorios1 = new relatorios;
    relatorios1->setWindowModality(Qt::ApplicationModal);
    relatorios1->show();
}

void MainWindow::imprimirEtiqueta1(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant barrasVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 4));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));


    imprimirEtiqueta(1, barrasVariant.toString(), descVariant.toString(), portugues.toString(precoVariant.toFloat()));


}
void MainWindow::imprimirEtiqueta3(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant barrasVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 4));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));


    imprimirEtiqueta(3, barrasVariant.toString(), descVariant.toString(), portugues.toString(precoVariant.toFloat()));


}
void MainWindow::imprimirEtiqueta(int quant, QString codBar, QString desc, QString preco){
    if (codBar == ""){
        QMessageBox::warning(this, "Erro", "código de barras inexistente");
        return;
    }
    // criar codigo de barras -----------
    QByteArray codBarBytes = codBar.toUtf8();
    const unsigned char* data = reinterpret_cast<const unsigned char*>(codBarBytes.constData());

    struct zint_symbol *barcode = ZBarcode_Create();
    if (!barcode) {
        qDebug() << "Erro ao criar o objeto de código de barras.";
        return;
    }

    // Definir o tipo de simbologia (Code128 neste caso)
    barcode->symbology = BARCODE_CODE128;
    barcode->output_options = BOLD_TEXT;

    // Definir os dados a serem codificados
    int error = ZBarcode_Encode(barcode, (unsigned char*)data, 0);
    if (error != 0) {
        qDebug() << "Erro ao codificar os dados: " << barcode->errtxt;
        ZBarcode_Delete(barcode);

    }

    // Gerar a imagem do código de barras e salvar como arquivo PNG ou GIF
    error = ZBarcode_Buffer(barcode, 0);
    if (error != 0) {
        qDebug() << "Erro ao criar o buffer da imagem: " << barcode->errtxt;
        ZBarcode_Delete(barcode);
        return ;
    }


    ZBarcode_Print(barcode, 0);
    // Limpar o objeto de código de barras
    ZBarcode_Delete(barcode);

    qDebug() << "Código de barras gerado com sucesso e salvo como out.png/out.gif";
    QImage codimage("out.gif");
    //  impressão ---------
    QPrinter printer;

    printer.setPageSize(QPageSize(QSizeF(80, 2000), QPageSize::Millimeter));
    // printer.setCopyCount(quant);

    QPrintDialog dialog(&printer, this);
    if(dialog.exec() == QDialog::Rejected) return;

    QPainter painter;
    painter.begin(&printer);

    // QByteArray cutCommand;
    // cutCommand.append(0x1D);
    // cutCommand.append('V');

    int ypos[2] = {5, 53};
    const int espacoEntreItens = 20;

    for(int i =0; i<quant; i++){


        if(i > 0){
            for(int j = 0; j < 2; j ++){
                ypos[j] = ypos[j] + 51;
            };
        }

        QRect descRect(0,ypos[0],145,32);
        QFont fontePainter = painter.font();
        fontePainter.setPointSize(10);
        painter.setFont(fontePainter);
        painter.drawText(descRect,Qt::TextWordWrap, desc);
        fontePainter.setBold(true);
        painter.setFont(fontePainter);
        painter.drawText(0, ypos[1], "Preço: R$" + portugues.toString(portugues.toFloat(preco), 'f', 2));
        fontePainter.setBold(false);
        painter.setFont(fontePainter);

        QRect codImageRect(140,ypos[0], 108,50);
        painter.drawImage(codImageRect, codimage);
        for(int j = 0; j < 2; j++) {
            ypos[j] = ypos[j] + espacoEntreItens;
        }


    }
    painter.end();






    }






void MainWindow::on_actionGerar_Relat_rio_CSV_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Salvar Arquivo CSV", "", "Arquivos CSV (*.csv)");

    if (fileName.isEmpty()) {
        // Se o usuário cancelar a seleção do arquivo, saia da função
        return;
    }

    // Abrindo o arquivo CSV para escrita
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Erro ao abrir o arquivo para escrita.";
        return;
    }
    QTextStream out(&file);

    if (!db.open()) {
        qDebug() << "Erro ao abrir o banco de dados botao csv.";
        return;
    }

    // Executando a consulta para recuperar os itens da tabela
    QSqlQuery query("SELECT * FROM produtos");
    out << "ID;Quant;Desc;Preço;CodBarra;NF\n";
    while (query.next()) {
        // Escrevendo os dados no arquivo CSV
        for (int i = 0; i < query.record().count(); ++i) {
            out << query.value(i).toString();
            if (i != query.record().count() - 1)
                out << ";"; // Adicionando ponto e vírgula para separar os campos
        }
        out << "\n"; // Adicionando uma nova linha após cada registro
    }

    // Fechando o arquivo e desconectando do banco de dados
    file.close();
    db.close();
}


void MainWindow::on_actionRealizar_Venda_triggered()
{
    venda *inserirVenda = new venda;
    //inserirVenda->setWindowModality(Qt::ApplicationModal);
    inserirVenda->show();
}


void MainWindow::on_Btn_AddProd_clicked()
{

    InserirProduto *addProdJanela = new InserirProduto;
    addProdJanela->show();
    connect(addProdJanela, &InserirProduto::codigoBarrasExistenteSignal,
            this, &MainWindow::atualizarTableviewComQuery);
    connect(addProdJanela, &InserirProduto::produtoInserido, this,
            &MainWindow::atualizarTableview);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Verificar se o evento é uma tecla pressionada no lineEdit
    if (obj == ui->Ledit_Pesquisa && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            ui->Btn_Pesquisa->click(); // Simula um clique no botão
            return true; // Evento tratado
        }
    }

    // Processar o evento padrão
    return QMainWindow::eventFilter(obj, event);
}
QString MainWindow::gerarNumero()
{
    QString number;
    do {
        number = QString("3562%1").arg(QRandomGenerator::global()->bounded(100000), 5, 10, QChar('0'));
    } while (generatedNumbers.contains(number));

    generatedNumbers.insert(number);
    // saveGeneratedNumber(number);

    return number;
}




void MainWindow::on_Tview_Produtos_customContextMenuRequested(const QPoint &pos)
{

    if(!ui->Tview_Produtos->currentIndex().isValid())
        return;

    QMenu menu(this);
    QMenu *imprimirMenu = new QMenu("Imprimir Etiqueta Código de Barra", this);

    menu.addAction(actionMenuAlterarProd);
    menu.addAction(actionMenuDeletarProd);
    menu.addAction(actionSetLocalProd);
    menu.addAction(actionVerProduto);
    imprimirMenu->setIcon(iconImpressora);
    imprimirMenu->addAction(actionMenuPrintBarCode1);
    imprimirMenu->addAction(actionMenuPrintBarCode3);
    menu.addMenu(imprimirMenu);

    menu.exec(ui->Tview_Produtos->viewport()->mapToGlobal(pos));
}




void MainWindow::on_actionTodos_Produtos_triggered()
{

    // // salva o arquivo
    QString fileName = QFileDialog::getSaveFileName(this, "Salvar PDF", QString(), "*.pdf");
     if (fileName.isEmpty())
         return;

     PDFexporter::exportarTodosProdutosParaPDF(fileName);

 }


void MainWindow::on_actionApenas_NF_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Salvar PDF", QString(), "*.pdf");
    if (fileName.isEmpty())
        return;

    PDFexporter::exportarNfProdutosParaPDF(fileName);



}


void MainWindow::on_actionConfig_triggered()
{
    Configuracao *configuracao = new Configuracao();
    configuracao->show();
    connect(configuracao, &Configuracao::alterouConfig, this,
            &MainWindow::atualizarConfigAcbr);
}


void MainWindow::on_Ledit_Pesquisa_textChanged(const QString &arg1)
{
    ui->Btn_Pesquisa->click();
}


void MainWindow::on_Btn_Clientes_clicked()
{


    Clientes *clientes = new Clientes;
    clientes->setWindowModality(Qt::ApplicationModal);
    clientes->show();
}

void MainWindow::atualizarTableviewComQuery(QString &query){
    model->setQuery(query);
}
void MainWindow::setLocalProd(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex idIndex = ui->Tview_Produtos->model()->index(selectedRow, 0);
        QModelIndex localIndex = ui->Tview_Produtos->model()->index(selectedRow, 14);

        int id = ui->Tview_Produtos->model()->data(idIndex).toInt();
        QString local = ui->Tview_Produtos->model()->data(localIndex).toString();
        QSqlQuery query;
        QStringList sugestoes;
        if(!db.isOpen()){
            db.open();
        }
        if (query.exec("SELECT DISTINCT local FROM produtos WHERE local IS NOT NULL AND TRIM(local) != ''")) {
            while (query.next()) {
                sugestoes << query.value(0).toString();
            }
        } else {
            qDebug() << "Erro ao executar query de locais:" << query.lastError().text();
        }

        LeditDialog dialog(this);
        dialog.setWindowTitle("Inserir Texto");
        dialog.setLabelText("Informe o local do produto:");
        dialog.setLineEditText(local);
        dialog.Ledit_info->setMaxLength(60);
        dialog.setCompleterSuggestions(sugestoes);

        if (dialog.exec() == QDialog::Accepted) {
            QString novoLocal = dialog.getLineEditText();
            qDebug() << "Novo local para ID" << id << ":" << novoLocal;

            query.prepare("UPDATE produtos SET local = :novolocal WHERE id = :id ");
            query.bindValue(":novolocal", novoLocal);
            query.bindValue(":id", id);
            if(!query.exec()){
                qDebug() << "falhou ao executar query update local";
            }
            atualizarTableview();
            emit localSetado();

        }
        db.close();
    }
}
int MainWindow::getIdProdSelected(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex idIndex = ui->Tview_Produtos->model()->index(selectedRow, 0);

        int id = ui->Tview_Produtos->model()->data(idIndex).toInt();
        return id;
    }
}
void MainWindow::verProd(){
    int id = getIdProdSelected();
    InfoJanelaProd *janelaProd = new InfoJanelaProd(this, id);
    janelaProd->show();
}

void MainWindow::on_actionDocumenta_o_triggered()
{
    HelpPage *page = new HelpPage(this, "");
    page->show();
}

void MainWindow::atualizarConfigAcbr(){

    auto acbr = AcbrManager::instance()->nfe();
    auto cnpj = ConsultaCnpjManager::instance()->cnpj();
    fiscalValues = Configuracao::get_All_Fiscal_Values();
    empresaValues = Configuracao::get_All_Empresa_Values();

    qDebug() << "=== CARREGANDO CONFIGURAÇÕES ACBR ===";
    QString caminhoXml = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                         "/xmlNf";
    QString caminhoEntradas = caminhoXml + "/entradas";
    QDir dir;
    if(!dir.exists(caminhoXml)){
        dir.mkpath(caminhoXml);
    }
    if(!dir.exists(caminhoEntradas)){
        dir.mkpath(caminhoEntradas);
    }

    // LIMPAR strings antes de usar
    auto cleanStr = [](const QString& str) -> std::string {
        std::string result = str.toStdString();
        result.erase(std::remove(result.begin(), result.end(), '\0'), result.end());
        return result.empty() ? "" : result;
    };

    std::string certificadoPath = cleanStr(fiscalValues.value("caminho_certificado"));
    std::string certificadoSenha = cleanStr(fiscalValues.value("senha_certificado"));
    std::string uf = cleanStr(empresaValues.value("estado_empresa"));
    std::string schemaPath = cleanStr(fiscalValues.value("caminho_schema"));
    std::string idCsc = cleanStr(fiscalValues.value("id_csc"));
    std::string csc = cleanStr(fiscalValues.value("csc"));
    std::string tpAmb = (fiscalValues.value("tp_amb") == "0" ? "1" : "0");
    qDebug() << "tpamb obtido: " << tpAmb;
    QString caminhoCompletoLogo = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                  "/imagens/" + QFileInfo(empresaValues.value("caminho_logo_empresa")).fileName();
    qDebug() << "Certificado:" << QString::fromStdString(certificadoPath);
    qDebug() << "Certificado antes: " << certificadoPath;
    qDebug() << "UF:" << QString::fromStdString(uf);
    qDebug() << "Schema:" << QString::fromStdString(schemaPath);
    qDebug() << "CaminhoCompletoLogo:" << caminhoCompletoLogo;


    //     // LIMPAR lista



    //     // CONFIGURAÇÕES PRINCIPAIS - DFe
    acbr->ConfigGravarValor("DFe", "ArquivoPFX", certificadoPath);
    acbr->ConfigGravarValor("DFe", "Senha", certificadoSenha);
    acbr->ConfigGravarValor("DFe", "UF", uf);
    acbr->ConfigGravarValor("DFe", "SSLHttpLib", "3");
    acbr->ConfigGravarValor("DFe", "SSLCryptLib", "1");
    acbr->ConfigGravarValor("DFe", "SSLXmlSignLib", "4");

    // //     // CONFIGURAÇÕES NFe
    acbr->ConfigGravarValor("NFe", "PathSchemas", schemaPath);
    acbr->ConfigGravarValor("NFe", "IdCSC", idCsc);
    acbr->ConfigGravarValor("NFe", "CSC", csc);
    acbr->ConfigGravarValor("NFe", "ModeloDF", "1");  // NFCe
    acbr->ConfigGravarValor("NFe", "VersaoDF", "3");
    acbr->ConfigGravarValor("NFe", "VersaoQRCode", "3");
    acbr->ConfigGravarValor("NFe", "FormaEmissao", "0");
    acbr->ConfigGravarValor("NFe", "Ambiente", tpAmb);
    acbr->ConfigGravarValor("NFE", "Download.PathDownload", caminhoEntradas.toStdString());
    //separar xml em pastas por nome da empresa
    acbr->ConfigGravarValor("NFe", "Download.SepararPorNome", "1");
    // // CONFIGURAÇÕES DE ARQUIVO

    acbr->ConfigGravarValor("NFe", "PathSalvar", caminhoXml.toStdString());
    acbr->ConfigGravarValor("NFe", "AdicionarLiteral", "1");
    acbr->ConfigGravarValor("NFe", "SepararPorCNPJ", "1");
    acbr->ConfigGravarValor("NFe", "SepararPorModelo", "1");
    acbr->ConfigGravarValor("NFe", "SepararPorAno", "1");
    acbr->ConfigGravarValor("NFe", "SepararPorMes", "1");
    acbr->ConfigGravarValor("NFe", "SalvarApenasNFeProcessadas", "1");
    acbr->ConfigGravarValor("NFe", "PathNFe", caminhoXml.toStdString());
    acbr->ConfigGravarValor("NFe", "SalvarEvento", "1");
    acbr->ConfigGravarValor("NFe", "PathEvento", caminhoXml.toStdString());

    acbr->ConfigGravarValor("NFe", "SalvarGer", "0");



    //sistema
    acbr->ConfigGravarValor("Sistema", "Nome", "QEstoqueLoja");
    acbr->ConfigGravarValor("Sistema", "Versao", VERSAO_QE);

    //     // CONFIGURAÇÕES DE CONEXÃO
    //     // nfce->ConfigGravarValor("NFe", "Timeout", "30000");
    //     // nfce->ConfigGravarValor("NFe", "Tentativas", "5");
    //     // nfce->ConfigGravarValor("NFe", "IntervaloTentativas", "1000");

        // // CONFIGURAÇÕES DANFE NFCe
    acbr->ConfigGravarValor("DANFE", "PathLogo", caminhoCompletoLogo.toStdString());

    //     // nfce->ConfigGravarValor("DANFENFCe", "TipoRelatorioBobina", "0");
    //     // nfce->ConfigGravarValor("DANFENFCe", "ImprimeItens", "1");
    //     // nfce->ConfigGravarValor("DANFENFCe", "ViaConsumidor", "1");
    //     // nfce->ConfigGravarValor("DANFENFCe", "FormatarNumeroDocumento", "1");
    acbr->ConfigGravar("");

    cnpj->ConfigGravarValor("ConsultaCNPJ", "Provedor", "3");
    cnpj->ConfigGravar("");
    qDebug() << "Configurações salvas no arquivo acbrlib.ini";

}


void MainWindow::on_Btn_Entradas_clicked()
{
    if(fiscalValues.value("emit_nf") == "1" && fiscalValues.value("tp_amb") == "1"){
        Entradas *entradas = new Entradas();
        entradas->show();
    }else{
        QMessageBox::warning(this, "Aviso", "Para visualizar as notas de entrada é "
                                            "necessário estar no ambiente 'Produção'.");
    }

}

