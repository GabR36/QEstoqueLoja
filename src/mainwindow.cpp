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
#include "util/consultacnpjmanager.h"
#include "entradas.h"
#include "util/manifestadordfe.h"
#include "util/mailmanager.h"
#include "janelaemailcontador.h"
#include "sobre.h"
#include "monitorfiscal.h"
#include "util/printutil.h"
#include "services/barcode_service.h"
#include "services/acbr_service.h"
#include "services/schemamigration_service.h"
#include "infra/databaseconnection_service.h"
#include "services/schemamigration_service.h"
#include "services/config_service.h"
#include "services/dfe_service.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setApplicationVersion(VERSAO_QE);

    model = new QSqlQueryModel(this);

    iniciarMigration();
    db = DatabaseConnection_service::db();

    produtoService = new Produto_Service();


    // configuracao do modelo e view produtos
    ui->Tview_Produtos->setModel(model);
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Quantidade"));
    model->setHeaderData(2, Qt::Horizontal, tr("Descrição"));
    model->setHeaderData(3, Qt::Horizontal, tr("Preço"));
    model->setHeaderData(4, Qt::Horizontal, tr("Código de Barras"));
    model->setHeaderData(5, Qt::Horizontal, tr("NF"));


    //carrega as configurações no DTO
    Config_service *confServ = new Config_service(this);
    configDTO = confServ->carregarTudo();

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
    // manifestdfe->consultarSePossivel();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::iniciarMigration(){

    SchemaMigration_service *schema = new SchemaMigration_service(this, ultimaVersaoSchema);
    //conexões de ações para update de versao schema
    connect(schema, &SchemaMigration_service::dbVersao6, this,
            &MainWindow::atualizarConfigAcbr);

    // schema->update();
    auto result = schema->update();
    if(!result.ok){
        QMessageBox::warning(this, "Erro migração", "Erro ao migrar banco de dados");
    }

}

void MainWindow::mostrarProdutoPorCodigoBarras(const QString &codigo)
{
    model = produtoService->getProdutoPeloCodigo(codigo);
    ui->Tview_Produtos->setModel(model);

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

void MainWindow::atualizarTableview()
{
    produtoService->listarProdutos(model);
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
        auto resultado = produtoService->deletar(productId);
        if(!resultado.ok){
            QMessageBox::warning(this,"Erro","Ocorreu um erro ao deletar o produto");
            return;
        }
        atualizarTableview();
    }
    else {
        // O usuário escolheu não deletar o produto
        qDebug() << "A exclusão do produto foi cancelada.";
    }
    }else{
        QMessageBox::warning(this,"Erro","Selecione um produto antes de tentar deletar!");
    }
}

void MainWindow::on_Btn_Pesquisa_clicked()
{
    QString inputText = ui->Ledit_Pesquisa->text();

    produtoService->pesquisar(inputText, model);

    if (!model) {
        QMessageBox::warning(this, "Erro", "Erro ao realizar a pesquisa.");
        return;
    }

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
    // alterar->setWindowModality(Qt::ApplicationModal);
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

    QString erro;

    QImage barcode = Barcode_service::gerarCodigoBarras(barrasVariant.toString(), &erro);
    if (barcode.isNull()) {
        QMessageBox::warning(this, "Erro", erro);
        return;
    }

    if (!PrintUtil::imprimirEtiquetas(this, 1, barcode, descVariant.toString(), precoVariant.toDouble(), &erro)) {
        QMessageBox::warning(this, "Erro", erro);
    }
}

void MainWindow::imprimirEtiqueta3(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant barrasVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 4));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));

    QString erro;

    QImage barcode = Barcode_service::gerarCodigoBarras(barrasVariant.toString(), &erro);
    if (barcode.isNull()) {
        QMessageBox::warning(this, "Erro", erro);
        return;
    }

    if (!PrintUtil::imprimirEtiquetas(this, 3, barcode, descVariant.toString(), precoVariant.toDouble(), &erro)) {
        QMessageBox::warning(this, "Erro", erro);
    }


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
            this, &MainWindow::mostrarProdutoPorCodigoBarras);
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

void MainWindow::setLocalProd()
{
    auto *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndexList selected = selectionModel->selectedIndexes();

    if (selected.isEmpty())
        return;

    int row = selected.first().row();

    QModelIndex idIndex    = ui->Tview_Produtos->model()->index(row, 0);
    QModelIndex localIndex = ui->Tview_Produtos->model()->index(row, 14);

    int id = ui->Tview_Produtos->model()->data(idIndex).toInt();
    QString localAtual = ui->Tview_Produtos->model()->data(localIndex).toString();

    // service
    auto sugestoes = produtoService->obterSugestoesLocal();

    LeditDialog dialog(this);
    dialog.setWindowTitle("Inserir Texto");
    dialog.setLabelText("Informe o local do produto:");
    dialog.setLineEditText(localAtual);
    dialog.Ledit_info->setMaxLength(60);
    dialog.setCompleterSuggestions(sugestoes);

    if (dialog.exec() == QDialog::Accepted) {
        QString novoLocal = dialog.getLineEditText();

        auto res = produtoService->atualizarLocalProduto(id, novoLocal);

        if (!res.ok) {
            QMessageBox::warning(this, "Erro", res.msg);
            return;
        }

        atualizarTableview();
        emit localSetado();
    }
}

QString MainWindow::getIdProdSelected(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex idIndex = ui->Tview_Produtos->model()->index(selectedRow, 0);

        QString id = ui->Tview_Produtos->model()->data(idIndex).toString();
        return id;
    }
}

void MainWindow::verProd(){
    QString id = getIdProdSelected();
    InfoJanelaProd *janelaProd = new InfoJanelaProd(this, id);
    janelaProd->show();
}

void MainWindow::on_actionDocumenta_o_triggered()
{
    HelpPage *page = new HelpPage(this, "");
    page->show();
}

void MainWindow::atualizarConfigAcbr(){
    qDebug() << " tentou atualizar acbr config";
        Acbr_service acbrService;
        auto res = acbrService.configurar(VERSAO_QE);
        if(!res.ok){
            if(res.erro != AcbrErro::NaoEmitindoNf){
                QMessageBox::critical(this, "Erro", res.msg);
                return;
            }

        }else{
            qDebug() << "Configurações salvas no arquivo acbrlib.ini";
        }

}


void MainWindow::on_Btn_Entradas_clicked()
{
    if(configDTO.emitNfFiscal && configDTO.tpAmbFiscal){
        Entradas *entradas = new Entradas();
        entradas->show();
        connect(entradas, &Entradas::produtoAdicionado, this,
                &MainWindow::atualizarTableview);
    }else{
        QMessageBox::warning(this, "Aviso", "Para visualizar as notas de entrada é "
                                            "necessário estar no ambiente 'Produção'.");
    }

}


void MainWindow::on_actionEnviar_triggered()
{
    JanelaEmailContador *janelaEmail = new JanelaEmailContador();
    janelaEmail->show();
}


void MainWindow::on_actionSobre_triggered()
{
    Sobre *sobre = new Sobre();
    sobre->show();
}


void MainWindow::on_actionMonitor_Fiscal_triggered()
{
    MonitorFiscal *monitor = new MonitorFiscal();
    monitor->show();
}

