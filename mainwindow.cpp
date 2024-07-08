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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    // criar banco de dados e tabela se não foi ainda.

    // verificar versao do esquema de banco de dados e aplicar as atualizacoes
    // necessárias

    db.setDatabaseName("estoque.db");
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados.";
    }

    // configuracao do modelo e view produtos
    ui->Tview_Produtos->setModel(model);
    model->setQuery("SELECT * FROM produtos ORDER BY id DESC");
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Quantidade"));
    model->setHeaderData(2, Qt::Horizontal, tr("Descrição"));
    model->setHeaderData(3, Qt::Horizontal, tr("Preço"));
    model->setHeaderData(4, Qt::Horizontal, tr("Código de Barras"));
    model->setHeaderData(5, Qt::Horizontal, tr("NF"));

    //teste float maior que 10000

    // QString a = "21320.3";
    // QString b = "7";
    // double all = a.toDouble() * b.toInt();

    // qDebug() << portugues.toString(all);
    // criar a versao 0 se o banco de dados estiver vazio
    QSqlQuery query;

    query.exec("CREATE TABLE produtos (id INTEGER PRIMARY KEY AUTOINCREMENT, quantidade INTEGER, descricao TEXT, preco DECIMAL(10,2), codigo_barras VARCHAR(20), nf BOOLEAN)");
    if (query.isActive()) {
        qDebug() << "Tabela produtos criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela produtos";
    }
    query.exec("CREATE TABLE vendas2 (id INTEGER PRIMARY KEY AUTOINCREMENT, cliente TEXT, data_hora DATETIME DEFAULT CURRENT_TIMESTAMP, total DECIMAL(10,2))");
    if (query.isActive()) {
        qDebug() << "Tabela de vendas2 criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela de vendas2: ";
    }
    query.exec("CREATE TABLE produtos_vendidos (id INTEGER PRIMARY KEY AUTOINCREMENT, id_produto INTEGER, id_venda INTEGER, quantidade INTEGER, preco_vendido DECIMAL(10,2), FOREIGN KEY (id_produto) REFERENCES produtos(id), FOREIGN KEY (id_venda) REFERENCES vendas2(id))");
    if (query.isActive()) {
        qDebug() << "Tabela de produtos_vendidos criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela de produtos_vendidos: ";
    }
    qDebug() << db.tables();

    // obter a versao do esquema do banco de dados
    int dbSchemaVersion;
    if (query.exec("PRAGMA user_version")) {
        if (query.next()) {
            dbSchemaVersion = query.value(0).toInt();
        }
    } else {
        qDebug() << "Failed to execute PRAGMA user_version:";
    }
    query.finish();
    qDebug() << dbSchemaVersion;

    // a versão mais recente do esquema do banco de dados
    int dbSchemaLastVersion = 2;

    while (dbSchemaVersion < dbSchemaLastVersion){
        // selecionar a atualizacao conforme a versao atual do banco de dados
        switch (dbSchemaVersion) {
        case 0:
        {
            // atualizar da versao 0 para a versao 1 do schema

            // comecar transacao
            if (!db.transaction()) {
                qDebug() << "Error: unable to start transaction";
            }

            QSqlQuery query;

            query.exec("ALTER TABLE vendas2 ADD COLUMN forma_pagamento VARCHAR(20)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN valor_recebido DECIMAL(10,2)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN troco DECIMAL(10,2)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN taxa DECIMAL(10,2)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN valor_final DECIMAL(10,2)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN desconto DECIMAL(10,2)");

            // converter as datas no formato dd-MM-yyyy para yyyy-MM-dd
            query.exec("UPDATE vendas2 "
                       "SET data_hora = strftime('%Y-%m-%d %H:%M:%S', substr(data_hora, 7, 4) || '-' || substr(data_hora, 4, 2) || '-' || substr(data_hora, 1, 2) || ' ' || substr(data_hora, 12, 8)) "
                       "WHERE substr(data_hora, 3, 1) = '-' AND substr(data_hora, 6, 1) = '-'");

            // colocar valores nas novas colunas
            query.exec("UPDATE vendas2 SET forma_pagamento = 'Não Sei', "
                       "valor_recebido = total, "
                       "troco = 0, taxa = 0, "
                       "valor_final = total,"
                       "desconto = 0");

            // mudar a versao para 1
            query.exec("PRAGMA user_version = 1");

            query.finish();

            // terminar transacao
            if (!db.commit()) {
                qDebug() << "Error: unable to commit transaction";
                db.rollback(); // Desfaz a transação
            }

            dbSchemaVersion = 1;

            break;
        }
        case 1:
        {
            // schema versao 1 atualizar para a versao 2

            // comecar transacao
            if (!db.transaction()) {
                qDebug() << "Error: unable to start transaction";
            }
            QSqlQuery query;

            query.exec("CREATE TABLE config (id INT AUTO_INCREMENT PRIMARY KEY, "
                       "key VARCHAR(255) NOT NULL UNIQUE, "
                       "value TEXT)");
            query.exec("INSERT INTO config (key, value) VALUES ('nome_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('endereco_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('telefone_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('cnpj_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('email_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('porcent_lucro', '40')");
            query.exec("INSERT INTO config (key, value) VALUES ('taxa_debito', '2')");
            query.exec("INSERT INTO config (key, value) VALUES ('taxa_credito', '3')");

            // normalizar dados existentes
            if (!query.exec("SELECT id, descricao FROM produtos")) {
                qDebug() << "Erro ao executar a consulta SQL:" << query.lastError().text();
            }

            // Iterar sobre os resultados da consulta
            while (query.next()) {
                int id = query.value(0).toInt();
                QString descricao = query.value(1).toString();

                // Normalizar a descrição
                QString descricaoNormalizada = normalizeText(descricao);

                // Atualizar a tabela produtos com a descrição normalizada
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE produtos SET descricao = :descricao WHERE id = :id");
                updateQuery.bindValue(":descricao", descricaoNormalizada);
                updateQuery.bindValue(":id", id);

                if (!updateQuery.exec()) {
                    qDebug() << "Erro ao atualizar a descrição do produto:" << updateQuery.lastError().text();
                    db.rollback();
                }
            }

            // mudar a versao para 2
            query.exec("PRAGMA user_version = 2");

            // terminar transacao
            if (!db.commit()) {
                qDebug() << "Error: unable to commit transaction";
                db.rollback(); // Desfaz a transação
            }

            dbSchemaVersion = 2;

            break;
        }
        }
    }
    qDebug() << dbSchemaVersion;
    qDebug() << db.tables();

    // mostrar na tabela da aplicaçao a tabela do banco de dados.
    ui->Tview_Produtos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    ui->groupBox->setVisible(false);
     ui->Ledit_Pesquisa->installEventFilter(this);
    atualizarTableview();
    db.close();
    //
    ui->Ledit_Barras->setFocus();
    // Selecionar a primeira linha da tabela
    QModelIndex firstIndex = model->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
     ui->Ledit_Desc->setMaxLength(120);

    // ajustar tamanho colunas
    // coluna descricao
    ui->Tview_Produtos->setColumnWidth(2, 750);
    // coluna quantidade
    ui->Tview_Produtos->setColumnWidth(1, 85);
    ui->Tview_Produtos->setColumnWidth(4,110);
    iconAlterarProduto.addFile(":/QEstoqueLOja/story-editor.svg");
    iconAddProduto.addFile(":/QEstoqueLOja/add-product.svg");
    iconBtnVenda.addFile(":/QEstoqueLOja/amarok-cart-view.svg");
    iconDelete.addFile(":/QEstoqueLOja/amarok-cart-remove.svg");
    iconPesquisa.addFile(":/QEstoqueLOja/edit-find.svg");
    iconBtnRelatorios.addFile(":/QEstoqueLOja/view-financial-account-investment-security.svg");
    iconImpressora.addFile(":/QEstoqueLOja/document-print.svg");


    ui->Btn_AddProd->setIcon(iconAddProduto);
    ui->Btn_Venda->setIcon(iconBtnVenda);
    ui->Btn_Alterar->setIcon(iconAlterarProduto);
    ui->Btn_Delete->setIcon(iconDelete);
    ui->Btn_Pesquisa->setIcon(iconPesquisa);
    ui->Btn_Relatorios->setIcon(iconBtnRelatorios);

    // validadores para os campos
    QDoubleValidator *DoubleValidador = new QDoubleValidator();
    ui->Ledit_Preco->setValidator(DoubleValidador);
    ui->Ledit_Quantidade->setValidator(DoubleValidador);


    // ações para menu de contexto tabela produtos
    actionMenuAlterarProd = new QAction(this);
    actionMenuDeletarProd = new QAction(this);

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



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::atualizarTableview(){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. atualizarTableView";
    }
    CustomDelegate *delegate = new CustomDelegate(this);
    ui->Tview_Produtos->setItemDelegate(delegate);
    model->setQuery("SELECT * FROM produtos ORDER BY id DESC");
    db.close();
}

void MainWindow::on_Btn_Enviar_clicked()
{
    QString quantidadeProduto, descProduto, precoProduto, barrasProduto;
    bool nfProduto;
    quantidadeProduto = ui->Ledit_Quantidade->text();
    descProduto = normalizeText(ui->Ledit_Desc->text());
    precoProduto = ui->Ledit_Preco->text();
    barrasProduto = ui->Ledit_Barras->text();
    nfProduto = ui->Check_Nf->isChecked();

    // Converta o texto para um número
    bool conversionOk;
    bool conversionOkQuant;
    double price = portugues.toDouble(precoProduto, &conversionOk);
    qDebug() << price;
    // quantidade precisa ser transformada com ponto para ser armazenada no db
    quantidadeProduto = QString::number(portugues.toFloat(quantidadeProduto, &conversionOkQuant));

    // Verifique se a conversão foi bem-sucedida e se o preço é maior que zero
    if (conversionOk && price >= 0)
    {
        if (conversionOkQuant){
            // guardar no banco de dados o valor notado em local da linguagem
            precoProduto = QString::number(price, 'f', 2);
            qDebug() << precoProduto;
            // verificar se o codigo de barras ja existe
            if (!verificarCodigoBarras()){
                // adicionar ao banco de dados
                if(!db.open()){
                    qDebug() << "erro ao abrir banco de dados. botao enviar.";
                }
                QSqlQuery query;

                query.prepare("INSERT INTO produtos (quantidade, descricao, preco, codigo_barras, nf) VALUES (:valor1, :valor2, :valor3, :valor4, :valor5)");
                query.bindValue(":valor1", quantidadeProduto);
                query.bindValue(":valor2", descProduto);
                query.bindValue(":valor3", precoProduto);
                query.bindValue(":valor4", barrasProduto);
                query.bindValue(":valor5", nfProduto);
                if (query.exec()) {
                    qDebug() << "Inserção bem-sucedida!";
                } else {
                    qDebug() << "Erro na inserção: ";
                }
                atualizarTableview();
                db.close();

                // limpar campos para nova inserçao
                ui->Ledit_Desc->clear();
                ui->Ledit_Quantidade->clear();
                ui->Ledit_Preco->clear();
                ui->Ledit_Barras->clear();
                ui->Check_Nf->setChecked(false);
                ui->Ledit_Barras->setFocus();
            }
        }
        else {
            // a quantidade é invalida
            QMessageBox::warning(this, "Erro", "Por favor, insira uma quantiade válida.");
            ui->Ledit_Quantidade->setFocus();
        }
    }
    else
    {
        // Exiba uma mensagem de erro se o preço não for válido
        QMessageBox::warning(this, "Erro", "Por favor, insira um preço válido.");
        ui->Ledit_Preco->clear();
    }


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
                result.append(c.toLower());
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
    qDebug() << "Palavras separadas:";
    for (const QString& palavra : palavras) {
        qDebug() << palavra;
    }

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
    QString productId = idVariant.toString();
    QString productQuant = portugues.toString(quantVariant.toFloat());
    QString productDesc = descVariant.toString();
    QString productPreco = portugues.toString(precoVariant.toFloat());
    QString productBarras = barrasVariant.toString();
    bool productNf = nfVariant.toBool();
    qDebug() << productId;
    qDebug() << productPreco;
    // criar janela
    AlterarProduto *alterar = new AlterarProduto;
    alterar->janelaPrincipal = this;
    alterar->idAlt = productId;
    alterar->TrazerInfo(productDesc, productQuant, productPreco, productBarras, productNf);
    alterar->setWindowModality(Qt::ApplicationModal);
    alterar->show();
    }else{
        QMessageBox::warning(this,"Erro","Selecione um produto antes de alterar!");
    }

}


void MainWindow::on_Btn_Venda_clicked()
{
    Vendas *vendas = new Vendas;
    vendas->janelaPrincipal = this;
    vendas->setWindowModality(Qt::ApplicationModal);
    vendas->show();
}



void MainWindow::on_Btn_Relatorios_clicked()
{
    relatorios *relatorios1 = new relatorios;
    relatorios1->setWindowModality(Qt::ApplicationModal);
    relatorios1->show();
}

void MainWindow::on_Ledit_Barras_returnPressed()
{
    // verificar se o codigo de barras existe
    verificarCodigoBarras();
    ui->Ledit_Desc->setFocus();
}

bool MainWindow::verificarCodigoBarras(){
    QString barrasProduto = ui->Ledit_Barras->text();
    // verificar se o codigo de barras ja existe
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

    if (barrasExiste){
        // codigo de barras existe, mostrar mensagem e
        // mostrar registro na tabela
        QMessageBox::warning(this, "Erro", "Esse código de barras já foi registrado.");
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. codigo de barras existente";
        }
        model->setQuery("SELECT * FROM produtos WHERE codigo_barras = " + barrasProduto);
        db.close();
        return true;
    }
    else{
        return false;
    }
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

        QRect descRect(0,ypos[0],145,34);
        QFont fontePainter = painter.font();
        fontePainter.setPointSize(10);
        painter.setFont(fontePainter);
        painter.drawText(descRect,Qt::TextWordWrap, desc);
        fontePainter.setBold(true);
        painter.setFont(fontePainter);
        painter.drawText(0, ypos[1], "Preço: R$" + portugues.toString(portugues.toFloat(preco), 'f', 2));
        fontePainter.setBold(false);
        painter.setFont(fontePainter);

        QRect codImageRect(155,ypos[0], 115,55);
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
    Vendas *janelaVenda = new Vendas;
   // MainWindow *janelaPrincipal;
   // QSqlDatabase db = QSqlDatabase::database();
    //explicit venda(QWidget *parent = nullptr);
    venda *inserirVenda = new venda;
    inserirVenda->janelaVenda = janelaVenda;
    inserirVenda->janelaPrincipal = this;
    inserirVenda->setWindowModality(Qt::ApplicationModal);
    inserirVenda->show();
}


void MainWindow::on_Btn_AddProd_clicked()
{
    ui->groupBox->setVisible(!ui->groupBox->isVisible());
    if(!ui->groupBox->isVisible()){
        ui->Tview_Produtos->setColumnWidth(2, 750);

    }else{
        ui->Tview_Produtos->setColumnWidth(2, 350);

    }
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
    imprimirMenu->setIcon(iconImpressora);
    imprimirMenu->addAction(actionMenuPrintBarCode1);
    imprimirMenu->addAction(actionMenuPrintBarCode3);
    menu.addMenu(imprimirMenu);

    menu.exec(ui->Tview_Produtos->viewport()->mapToGlobal(pos));
}



void MainWindow::on_Btn_GerarCodBarras_clicked()
{
    ui->Ledit_Barras->setText(gerarNumero());

}


void MainWindow::on_actionTodos_Produtos_triggered()
{

        // salva o arquivo
        QString fileName = QFileDialog::getSaveFileName(this, "Salvar PDF", QString(), "*.pdf");
        if (fileName.isEmpty())
            return;

        if (!db.open()) {
            qDebug() << "nao abriu bd";
            return;
        }

        QPdfWriter writer(fileName);
        writer.setPageSize(QPageSize(QPageSize::A4));
        QPainter painter(&writer);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        painter.setFont(QFont("Arial", 10, QFont::Bold));

        // Determinar a altura de uma linha e o espaço disponível na página
        int lineHeight = 300; // Altura de uma linha
        int availableHeight = writer.height(); // Altura disponível na página
        int startY = 1500; // Define a coordenada Y inicial

        // Desenha os dados da tabela no PDF
        QImage logo(":/QEstoqueLOja/mkaoyvbl.png");
        painter.drawImage(QRect(100, 100, 2000, 400), logo);
        painter.drawText(500, 1000,       "Dados da Tabela Produtos:");
        painter.drawText(1000, 1500,       "ID");
        painter.drawText(1600, 1500, "Quantidade");
        painter.drawText(3000, 1500, "Descrição");
        painter.drawText(8500, 1500, "Preço R$");

        QSqlQuery query("SELECT * FROM produtos");

        int row2 = 0;
        double sumData4 = 0.0;

        while(query.next()){
             QString data2 = query.value(1).toString(); // quant

              QString data4 = query.value(3).toString(); // preco

            // double preco = portugues.toDouble(data4.toString());
              double valueData4 = data4.toDouble() * data2.toDouble(); // Converte o valor para double
             sumData4 += valueData4; // Adiciona o valor à soma total

            ++row2;
        };
        // float a = 107926.0 + 0.4;
        // qDebug() << QString::number(a);

        painter.drawText(5000, 1000,"total R$:" + portugues.toString(sumData4,'f',2) );
        painter.drawText(8000, 1000,"total itens:" + QString::number(row2));

        QSqlQuery query2("SELECT * FROM produtos");




        int row = 1;
        //  double sumData4 = 0.0;

        QFontMetrics metrics(painter.font());
        while (query2.next()) {
            QString data1 = query2.value(0).toString(); // id
            QString data2 = query2.value(1).toString(); // quant
            QString data3 = query2.value(2).toString(); // desc
            QString data4 = query2.value(3).toString(); // preco
            QRect rect = metrics.boundingRect(QRect(0, 0, 4000, lineHeight), Qt::TextWordWrap, data3);
            int textHeight = rect.height();

            // Verifica se há espaço suficiente na página atual para desenhar outra linha
            if (startY + lineHeight * row > availableHeight) {
                // Se não houver, inicie uma nova página
                writer.newPage();
                startY = 100; // Reinicie a coordenada Y inicial
                row = 1; // Reinicie o contador de linha
            }

            // Desenhe os dados na página atual
            painter.drawText(QRect(1000, startY + lineHeight * row, 4000, textHeight), data1); //stary = 1500
            painter.drawText(QRect(1600, startY + lineHeight * row, 4000, textHeight), portugues.toString(data2.toDouble()));
            painter.drawText(QRect(3000, startY + lineHeight * row, 4000, textHeight), Qt::TextWordWrap, data3); // data3 com quebra de linha
            painter.drawText(QRect(8500, startY + lineHeight * row, 4000, textHeight), portugues.toString(data4.toDouble()));

            startY += textHeight;

            ++row;
        }

        // // Desenha a quantidade de itens e a soma dos preços apenas na primeira página
        // painter.drawText(4000, 1000, "Quantidade de Itens: " + QString::number(totalItems));
        // painter.drawText(4000, 1100, "Soma dos preços: R$ " + QString::number(sumData4));

        painter.end();

        db.close();

        // Abre o PDF após a criação
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));


 }


void MainWindow::on_actionApenas_NF_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Salvar PDF", QString(), "*.pdf");
    if (fileName.isEmpty())
        return;

    if (!db.open()) {
        qDebug() << "nao abriu bd";
        return;
    }

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    QPainter painter(&writer);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setFont(QFont("Arial", 10, QFont::Bold));

    // Determinar a altura de uma linha e o espaço disponível na página
    int lineHeight = 300; // Altura de uma linha
    int availableHeight = writer.height(); // Altura disponível na página
    int startY = 1500; // Define a coordenada Y inicial

    // Desenha os dados da tabela no PDF
    QImage logo(":/QEstoqueLOja/mkaoyvbl.png");
    painter.drawImage(QRect(100, 100, 2000, 400), logo);
    painter.drawText(500, 1000,       "Dados da Tabela Produtos:");
    painter.drawText(1000, 1500,       "ID");
    painter.drawText(1600, 1500, "Quantidade");
    painter.drawText(3000, 1500, "Descrição");
    painter.drawText(8500, 1500, "Preço R$");

    QSqlQuery query("SELECT * FROM produtos");

    int row2 = 1;
    int rowNF = 1;
    float sumData4 = 0.0;
    while(query.next()){
        QString data2 = query.value(1).toString(); // quant
        QString data4 = query.value(3).toString(); // preco
        int variantNf = query.value(5).toInt();

        if(variantNf == 1){



        // double preco = portugues.toDouble(data4.toString());
        float valueData4 = data4.toDouble() * data2.toDouble(); // Converte o valor para double
        sumData4 += valueData4; // Adiciona o valor à soma total
        ++rowNF;

        }

        ++row2;
    };

    painter.drawText(5000, 1000,"total R$:" + portugues.toString(sumData4,'f',2));
    painter.drawText(8000, 1000,"total itens:" + QString::number( rowNF));

    QSqlQuery query2("SELECT * FROM produtos");




    int row = 1;
    //  double sumData4 = 0.0;

    QFontMetrics metrics(painter.font());
    while (query2.next()) {
        QString data1 = query2.value(0).toString(); // id
        QString data2 = query2.value(1).toString(); // quant
        QString data3 = query2.value(2).toString(); // desc
        QString data4 = query2.value(3).toString(); // preco
        QString nf = query2.value(5).toString();



            QRect rect = metrics.boundingRect(QRect(0, 0, 4000, lineHeight), Qt::TextWordWrap, data3);
            int textHeight = rect.height();

            // Verifica se há espaço suficiente na página atual para desenhar outra linha
            if (startY + lineHeight * row > availableHeight) {
                // Se não houver, inicie uma nova página
                writer.newPage();
                startY = 100; // Reinicie a coordenada Y inicial
                row = 1; // Reinicie o contador de linha
            }

             if(nf == "1"){

            // Desenhe os dados na página atual
            painter.drawText(QRect(1000, startY + lineHeight * row, 4000, textHeight), data1); //stary = 1500
            painter.drawText(QRect(1600, startY + lineHeight * row, 4000, textHeight), portugues.toString(data2.toDouble()));
            painter.drawText(QRect(3000, startY + lineHeight * row, 4000, textHeight), Qt::TextWordWrap, data3); // data3 com quebra de linha
            painter.drawText(QRect(8500, startY + lineHeight * row, 4000, textHeight), portugues.toString(data4.toDouble()));

            startY += textHeight;

            ++row;
             }

    }
    qDebug() << "row = " + row;

    painter.end();

    db.close();

    // Abre o PDF após a criação
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));



}


void MainWindow::on_actionConfig_triggered()
{
    Config *configuracao = new Config();
    configuracao->show();
}

