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
#include "vendas.h"
#include <QDoubleValidator>
#include "relatorios.h"
#include "venda.h"
#include <QIntValidator>


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
    int dbSchemaLastVersion = 1;

    while (dbSchemaVersion < dbSchemaLastVersion){
        // selecionar a atualizacao conforme a versao atual do banco de dados
        switch (dbSchemaVersion) {
        case 0:
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

            // terminar transacao
            if (!db.commit()) {
                qDebug() << "Error: unable to commit transaction";
                db.rollback(); // Desfaz a transação
            }

            dbSchemaVersion = 1;

            break;
        }
    }

    // mostrar na tabela da aplicaçao a tabela do banco de dados.
    ui->Tview_Produtos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    ui->groupBox->setVisible(false);
     ui->Ledit_Pesquisa->installEventFilter(this);
    atualizarTableview();
    QSqlDatabase::database().close();
    //
    ui->Ledit_Barras->setFocus();
    // Selecionar a primeira linha da tabela
    QModelIndex firstIndex = model->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);

    // ajustar tamanho colunas
    // coluna descricao
    ui->Tview_Produtos->setColumnWidth(2, 750);
    // coluna quantidade
    ui->Tview_Produtos->setColumnWidth(1, 85);
    ui->Tview_Produtos->setColumnWidth(4,110);
    ui->Btn_AddProd->setIcon(QIcon(":/QEstoqueLOja/addblack.svg"));
    ui->Btn_Venda->setIcon(QIcon(":/QEstoqueLOja/shopping.png"));
    ui->Btn_Alterar->setIcon(QIcon(":/QEstoqueLOja/rebase.png"));
    ui->Btn_Delete->setIcon(QIcon(":/QEstoqueLOja/delete.png"));
    ui->Btn_Pesquisa->setIcon(QIcon(":/QEstoqueLOja/search.png"));
    ui->Btn_Relatorios->setIcon(QIcon(":/QEstoqueLOja/monitoring.svg"));

    // validadores para os campos
    QDoubleValidator *DoubleValidador = new QDoubleValidator();
    QIntValidator *IntValidador = new QIntValidator();
    ui->Ledit_Preco->setValidator(DoubleValidador);
    ui->Ledit_Quantidade->setValidator(IntValidador);
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
    ui->Tview_Produtos->setModel(model);
    db.close();
}

void MainWindow::on_Btn_Enviar_clicked()
{
    QString quantidadeProduto, descProduto, precoProduto, barrasProduto;
    bool nfProduto;
    quantidadeProduto = ui->Ledit_Quantidade->text();
    descProduto = ui->Ledit_Desc->text();
    precoProduto = ui->Ledit_Preco->text();
    barrasProduto = ui->Ledit_Barras->text();
    nfProduto = ui->Check_Nf->isChecked();

    // Converta o texto para um número
    bool conversionOk;
    bool conversionOkQuant;
    double price = portugues.toDouble(precoProduto, &conversionOk);
    qDebug() << price;
    // quantidade precisa ser transformada com ponto para ser armazenada no db
    quantidadeProduto = QString::number(portugues.toInt(quantidadeProduto, &conversionOkQuant));

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
                QSqlDatabase::database().close();

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
        QSqlDatabase::database().close();
    }
    else {
        // O usuário escolheu não deletar o produto
        qDebug() << "A exclusão do produto foi cancelada.";
    }
}


void MainWindow::on_Btn_Pesquisa_clicked()
{
    QString pesquisa = ui->Ledit_Pesquisa->text();
    // mostrar na tableview a consulta
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao pesquisar.";
    }
    model->setQuery("SELECT * FROM produtos WHERE descricao LIKE '%" + pesquisa + "%' ORDER BY id DESC");
    ui->Tview_Produtos->setModel(model);
    QSqlDatabase::database().close();
}




void MainWindow::on_Btn_Alterar_clicked()
{
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
    QString productQuant = quantVariant.toString();
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
        ui->Tview_Produtos->setModel(model);
        db.close();
        return true;
    }
    else{
        return false;
    }
}


void MainWindow::on_actionGerar_Relat_rio_PDF_triggered()
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
    double sumData4 = 0.0;
    while(query.next()){
        QString data2 = query.value(1).toString(); // quant

        QString data4 = query.value(3).toString(); // preco
        double valueData4 = data4.toDouble() * data2.toInt(); // Converte o valor para double
        sumData4 += valueData4; // Adiciona o valor à soma total


        ++row2;
    };

    painter.drawText(5000, 1000,"total R$:" + QString::number( sumData4));
    painter.drawText(8000, 1000,"total itens:" + QString::number( row2));

    QSqlQuery query2("SELECT * FROM produtos");




    int row = 1;
    //  double sumData4 = 0.0;


    while (query2.next()) {
        QString data1 = query2.value(0).toString(); // id
        QString data2 = query2.value(1).toString(); // quant
        QString data3 = query2.value(2).toString(); // desc
        QString data4 = query2.value(3).toString(); // preco

        // Verifica se há espaço suficiente na página atual para desenhar outra linha
        if (startY + lineHeight * row > availableHeight) {
            // Se não houver, inicie uma nova página
            writer.newPage();
            startY = 100; // Reinicie a coordenada Y inicial
            row = 1; // Reinicie o contador de linha
        }

        // Desenhe os dados na página atual
        painter.drawText(1000, startY + lineHeight * row, data1);
        painter.drawText(1600, startY + lineHeight * row, data2);
        painter.drawText(3000, startY + lineHeight * row, data3);
        painter.drawText(8500, startY + lineHeight * row, data4);

        double valueData4 = data4.toDouble(); // Converte o valor para double
        sumData4 += valueData4; // Adiciona o valor à soma total

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
                out << ";"; // Adicionando vírgula para separar os campos
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

void MainWindow::on_pushButton_clicked()
{
    ui->Ledit_Barras->setText(gerarNumero());
}

