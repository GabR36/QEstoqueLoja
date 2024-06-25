#include "vendas.h"
#include "ui_vendas.h"
#include <QSqlQueryModel>
#include "venda.h"
#include <QDate>
#include <QtSql>
#include <QMessageBox>
#include <QMenu>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>


Vendas::Vendas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Vendas)
{
    ui->setupUi(this);
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao venda.";
    }
    // configuracao modelos e views tabelas vendas e produtosvendidos
    modeloVendas2->setQuery("SELECT * FROM vendas2 ORDER BY id DESC");
    ui->Tview_Vendas2->setModel(modeloVendas2);
    modeloVendas2->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modeloVendas2->setHeaderData(1, Qt::Horizontal, tr("Cliente"));
    modeloVendas2->setHeaderData(2, Qt::Horizontal, tr("Data e Hora"));
    modeloVendas2->setHeaderData(3, Qt::Horizontal, tr("Total Produtos"));
    modeloVendas2->setHeaderData(4, Qt::Horizontal, tr("Forma de Pagamento"));
    modeloVendas2->setHeaderData(5, Qt::Horizontal, tr("Valor Recebido"));
    modeloVendas2->setHeaderData(6, Qt::Horizontal, tr("Troco"));
    modeloVendas2->setHeaderData(7, Qt::Horizontal, tr("Taxa"));
    modeloVendas2->setHeaderData(8, Qt::Horizontal, tr("Valor Final"));
    modeloVendas2->setHeaderData(9, Qt::Horizontal, tr("Desconto"));

    modeloProdVendidos->setQuery("SELECT * FROM produtos_vendidos");
    ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
    modeloProdVendidos->setHeaderData(0, Qt::Horizontal, tr("Descrição"));
    modeloProdVendidos->setHeaderData(1, Qt::Horizontal, tr("Quantidade"));
    modeloProdVendidos->setHeaderData(2, Qt::Horizontal, tr("Preço Vendido"));
    db.close();
    ui->Tview_Vendas2->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    ui->Tview_ProdutosVendidos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    atualizarTabelas();

    // Selecionar a primeira linha das tabelas
    QModelIndex firstIndex = modeloVendas2->index(0, 0);
    ui->Tview_Vendas2->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
    QModelIndex firstIndex2 = modeloProdVendidos->index(0, 0);
    // Obter o modelo de seleção da tabela
    QItemSelectionModel *selectionModel = ui->Tview_Vendas2->selectionModel();
    // Conectar o sinal de seleção ao slot personalizado
    connect(selectionModel, &QItemSelectionModel::selectionChanged,this, &Vendas::handleSelectionChange);
    // ajustar tamanho colunas
    // coluna data
    ui->Tview_Vendas2->setColumnWidth(2, 150);
    // coluna cliente
    ui->Tview_Vendas2->setColumnWidth(1, 100);
    // coluna descricao
    ui->Tview_Vendas2->setColumnWidth(4, 110);
    ui->Tview_Vendas2->setColumnWidth(5, 100);
    ui->Tview_Vendas2->setColumnWidth(8, 80);
    ui->Tview_ProdutosVendidos->setColumnWidth(0, 400);
    // coluna quantidade
    ui->Tview_ProdutosVendidos->setColumnWidth(1, 85);


    // conectar banco de dados para consultar as datas mais antigas e mais novas das vendas
    // para mostrar nos qdateedits
    if (!db.open()) {
        qDebug() << "Error: Could not connect to database.";
    }

    QSqlQuery query;
    QPair<QDate, QDate> dateRange;
    // data antiga
    query.exec("SELECT MIN(data_hora) FROM vendas2");
    if (query.next()) {
        dateRange.first = query.value(0).toDate();
    }

    // data nova
    query.exec("SELECT MAX(data_hora) FROM vendas2");
    if (query.next()) {
        dateRange.second = query.value(0).toDate();
    }
    query.finish();

    qDebug() << dateRange;

    ui->DateEdt_De->setDate(dateRange.first);
    ui->DateEdt_Ate->setDate(dateRange.second);

    db.close();
    ui->Tview_Vendas2->selectionModel()->select(firstIndex, QItemSelectionModel::Select);


}

Vendas::~Vendas()
{
    delete ui;
}

void Vendas::on_Btn_InserirVenda_clicked()
{
    venda *inserirVenda = new venda;
    inserirVenda->janelaVenda = this;
    inserirVenda->janelaPrincipal = janelaPrincipal;
    inserirVenda->setWindowModality(Qt::ApplicationModal);
    inserirVenda->show();
}

void Vendas::atualizarTabelas(){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao venda.";
    }
    modeloVendas2->setQuery("SELECT * FROM vendas2 ORDER BY id DESC");

    modeloProdVendidos->setQuery("SELECT * FROM produtos_vendidos");

    db.close();

    QModelIndex firstIndex = modeloVendas2->index(0, 0);


    ui->Tview_Vendas2->selectionModel()->select(firstIndex, QItemSelectionModel::Select);

    // Definir a primeira linha como a linha atual
//    selectionModel->setCurrentIndex(topLeft, QItemSelectionModel::NoUpdate);

}
 QStringList Vendas::getProdutosVendidos( QString idVenda){

    QStringList produtosVendidos;
    QSqlDatabase db2 = QSqlDatabase::database();
    QSqlQuery query;

    if (!db2.open()) {
        qDebug() << "Banco de dados db2 não abriu";
        return produtosVendidos;
    }

    query.prepare("SELECT produtos.descricao, produtos_vendidos.quantidade, produtos_vendidos.preco_vendido "
                  "FROM produtos_vendidos "
                  "JOIN produtos ON produtos_vendidos.id_produto = produtos.id "
                  "WHERE produtos_vendidos.id_venda = :id_venda");
    query.bindValue(":id_venda", idVenda);

    if (!query.exec()) {
        qDebug() << "Erro ao executar a query:" << query.lastError().text();
        return produtosVendidos;
    }

    while (query.next()) {
        QString descricao = query.value(0).toString();
        QString quantidade = QString::number(query.value(1).toInt());
        QString precoVendido = QString::number(query.value(2).toDouble(), 'f', 2);

        // Adiciona cada campo individualmente à lista
        produtosVendidos.append(descricao);
        produtosVendidos.append(quantidade);
        produtosVendidos.append(precoVendido);
    }
    db2.close();


    return produtosVendidos;
}

void Vendas::handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected) {
    // Este slot é chamado sempre que a seleção na tabela muda
    Q_UNUSED(deselected);

    qDebug() << "Registro(s) selecionado(s):";

    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. handleselectionchange";
    }
    QModelIndex selectedIndex = selected.indexes().first();
    QVariant idVariant = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 0));
    QString productId = idVariant.toString();
    modeloProdVendidos->setQuery("SELECT produtos.descricao, produtos_vendidos.quantidade, produtos_vendidos.preco_vendido FROM produtos_vendidos JOIN produtos ON produtos_vendidos.id_produto = produtos.id WHERE id_venda = " + productId);
    ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
    db.close();


}

void Vendas::LabelLucro(QString whereQuery){
    // colocar valores nos labels de lucro etc
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. labelLucro";
    }
    QSqlQuery query;
    float total = 0;
    int quantidadeVendas = 0;
    if (query.exec("SELECT SUM(valor_final) FROM vendas2 " + whereQuery)) {
        while (query.next()) {
            total = query.value(0).toFloat();
        }
    }
    if (query.exec("SELECT COUNT(*) FROM vendas2 " + whereQuery)) {
        while (query.next()) {
            quantidadeVendas = query.value(0).toInt();
        }
    }
    query.finish();
    // pegar a porcentagem de lucro das configuracoes
    float porcentLucro = 0;
    if (query.exec("SELECT value FROM config WHERE key = 'porcent_lucro'")){
        while (query.next()) {
            qDebug() << query.value(0).toString();
            porcentLucro = query.value(0).toFloat()/100;
        }
    }

    qDebug() << porcentLucro;
    // formula lucro em funcao de total, porcentagem de lucro e quantidade de vendas
    float lucro = total*porcentLucro/(1 + porcentLucro);
    ui->Lbl_Total->setText(portugues.toString(total, 'f', 2));
    ui->Lbl_Lucro->setText(portugues.toString(lucro, 'f', 2));
    ui->Lbl_Quantidade->setText(portugues.toString(quantidadeVendas));
    db.close();
}

void Vendas::LabelLucro(){
    // para ser chamada sem argumentos
    LabelLucro(QString());
}


void Vendas::on_Btn_DeletarVenda_clicked()
{
    if(ui->Tview_Vendas2->currentIndex().isValid()){
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Vendas2->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 0));
    QVariant valorVariant = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 3));
    QString productId = idVariant.toString();
    QString productValor = valorVariant.toString();

    // Cria uma mensagem de confirmação
    QMessageBox::StandardButton resposta;
    resposta = QMessageBox::question(
        nullptr,
        "Confirmação",
        "Tem certeza que deseja excluir a venda:\n\n"
        "id: " + productId + "\n"
                          "Valor total: " + productValor,
        QMessageBox::Yes | QMessageBox::No
        );
    // Verifica a resposta do usuário
    if (resposta == QMessageBox::Yes) {

        // remover registro do banco de dados
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. botao deletar.";
        }
        QSqlQuery query;

        // pegar os ids dos produtos e quantidades para adicionar depois
        query.prepare("SELECT id_produto, quantidade FROM produtos_vendidos WHERE id_venda = :valor1");
        query.bindValue(":valor1", productId);
        if (query.exec()) {
            qDebug() << "query bem-sucedido!";
        } else {
            qDebug() << "Erro no query: ";
        }
        while (query.next()){
            QString idProduto = query.value(0).toString();
            QString quantProduto = query.value(1).toString();
            qDebug() << idProduto;
            qDebug() <<  quantProduto;
            QSqlQuery updatequery;
            updatequery.prepare("UPDATE produtos SET quantidade = quantidade + :quantidade WHERE id = :id");
            updatequery.bindValue(":id", idProduto);
            updatequery.bindValue(":quantidade", quantProduto);
            if (updatequery.exec()) {
                qDebug() << "query UPDATE bem-sucedido!";
            } else {
                qDebug() << "Erro no query UPDATE: ";
            }
        }


        // deletar a venda
        query.prepare("DELETE FROM vendas2 WHERE id = :valor1");
        query.bindValue(":valor1", productId);
        if (query.exec()) {
            qDebug() << "Delete bem-sucedido!";
        } else {
            qDebug() << "Erro no Delete: ";
        }
        // deletar os produtos vendidos
        query.prepare("DELETE FROM produtos_vendidos WHERE id_venda = :valor1");
        query.bindValue(":valor1", productId);
        if (query.exec()) {
            qDebug() << "Delete bem-sucedido!";
        } else {
            qDebug() << "Erro no Delete: ";
        }      
        atualizarTabelas();
        db.close();
    }
    else {
        // O usuário escolheu não deletar o produto
        qDebug() << "A exclusão da venda foi cancelada.";
    }
    }else{
        QMessageBox::warning(this,"Erro","Selecione uma venda antes de tentar deletar!");
    }


}

void Vendas::on_DateEdt_De_dateChanged(const QDate &date)
{
    // precisa adicionar um dia para ele contar o dia todo
    QString ate = ui->DateEdt_Ate->date().addDays(1).toString("yyyy-MM-dd");
    QString de = date.toString("yyyy-MM-dd");
    qDebug() << de;
    qDebug() << ate;
    filtrarData(de, ate);
}


void Vendas::on_DateEdt_Ate_dateChanged(const QDate &date)
{
    QString de = ui->DateEdt_De->date().toString("yyyy-MM-dd");
    // precisa adicionar um dia para ele contar o dia todo
    QString ate = date.addDays(1).toString("yyyy-MM-dd");
    qDebug() << de;
    qDebug() << ate;
    filtrarData(de, ate);
}

void Vendas::filtrarData(QString de, QString ate){
    QString whereQuery = QString("WHERE data_hora BETWEEN '%1' AND '%2'").arg(de, ate);
    LabelLucro(whereQuery);
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. filtrarData";
    }
    modeloVendas2->setQuery("SELECT * FROM vendas2 " + whereQuery + " ORDER BY id DESC");
    db.close();
    ui->Tview_Vendas2->selectionModel()->select(QModelIndex(modeloVendas2->index(0, 0)), QItemSelectionModel::Select);

}



void Vendas::on_Tview_Vendas2_customContextMenuRequested(const QPoint &pos)
{    if(!ui->Tview_Vendas2->currentIndex().isValid())
        return;
    QModelIndexList selectedRows = ui->Tview_Vendas2->selectionModel()->selectedRows();
    QString cellValue;
    if (!selectedRows.isEmpty()) {
        // Pega o primeiro índice selecionado (caso múltiplas linhas possam ser selecionadas)
        QModelIndex selectedIndex = selectedRows.first();

        // Obtém o índice da célula na coluna 0 da linha selecionada
        QModelIndex columnIndex = ui->Tview_Vendas2->model()->index(selectedIndex.row(), 0);

        // Obtém o valor da célula como uma QString
         cellValue = ui->Tview_Vendas2->model()->data(columnIndex).toString();

        // Mostra o valor em uma mensagem

    } else {
        QMessageBox::warning(this, "Aviso", "Nenhuma linha selecionada.");
    }

    QMenu menu(this);

    actionMenuDeletarVenda = new QAction();
    actionMenuDeletarVenda->setText("Deletar Venda");
    actionMenuDeletarVenda->setIcon(janelaPrincipal->iconDelete);
    connect(actionMenuDeletarVenda,SIGNAL(triggered(bool)),this,SLOT(on_Btn_DeletarVenda_clicked()));

    actionImprimirRecibo = new QAction();
    actionImprimirRecibo->setText("Imprimir Recibo da Venda");
    actionImprimirRecibo->setIcon(janelaPrincipal->iconImpressora);
    QObject::connect(actionImprimirRecibo, &QAction::triggered, [&]() {
        imprimirReciboVenda(cellValue); // Chama nossa função com o parâmetro
    });


    menu.addAction(actionMenuDeletarVenda);
    menu.addAction(actionImprimirRecibo);



    menu.exec(ui->Tview_Vendas2->viewport()->mapToGlobal(pos));
}

bool Vendas::imprimirReciboVenda(QString idVenda){
    QLocale portugues2;

    QSqlDatabase db2 = QSqlDatabase::database();
    if(!db2.open()){
        qDebug() << "erro bancodedados";
    }
    QSqlQuery query;
    QPrinter printer;

    printer.setPageSize(QPageSize(QSizeF(80, 2000), QPageSize::Millimeter));// Tamanho do papel
    // printer.pageLayout().setPageSize(customPageSize);
    printer.setFullPage(true); // Utilizar toda a página        QPrintDialog dialog(&printer, this);

    QPrintDialog dialog(&printer);
   if(dialog.exec() == QDialog::Rejected) return 0;

    QPainter painter;
    painter.begin(&printer);
    QFont font = painter.font();
    font.setPointSize(8);
    font.setBold(true);
    painter.setFont(font);

    QString nomeEmpresa = "Padrão"; // pega os dados da configuração;
    QString enderecoEmpresa = "Padrão";
    QString cnpjEmpresa = "";
    QString telEmpresa = "";
    QString cliente,total,forma_pagamento,valor_recebido,troco,taxa,valor_final,desconto;
    QDateTime dataVenda;
    if (query.exec("SELECT value FROM config WHERE key = 'nome_empresa'")){
        while (query.next()) {
            nomeEmpresa = query.value(0).toString();
        }
    };
    if (query.exec("SELECT value FROM config WHERE key = 'endereco_empresa'")){
        while (query.next()) {
            enderecoEmpresa = query.value(0).toString();
        }
    };
    if (query.exec("SELECT value FROM config WHERE key = 'cnpj_empresa'")){
        while (query.next()) {
            cnpjEmpresa = query.value(0).toString();
        }
    };
    if (query.exec("SELECT value FROM config WHERE key = 'telefone_empresa'")){
        while (query.next()) {
            telEmpresa = query.value(0).toString();
        }// dataglobal , clienteglobal, totalglobal, desconte, forma de pagamento,recebido, taca e valorfinal?
    };
    query.prepare("SELECT cliente, data_hora,total,forma_pagamento,valor_recebido,troco,taxa,valor_final,desconto FROM vendas2 WHERE id = :id_venda");
    query.bindValue(":id_venda", idVenda);
    if(query.exec()){
        while (query.next()) {
                cliente = query.value("cliente").toString();
                dataVenda = query.value("data_hora").toDateTime();
                total = query.value("total").toString();
                forma_pagamento = query.value("forma_pagamento").toString();
                valor_recebido = query.value("valor_recebido").toString();
                troco = query.value("troco").toString();
                taxa = query.value("taxa").toString();
                valor_final = query.value("valor_final").toString();
                desconto = query.value("desconto").toString();

            }
    }else{qDebug() << "erro query venda2 imprimir";}





    int yPos = 30; // Posição inicial para começar a desenhar o texto
    int xPos = 0;
    const int yPosPrm = 10; // Posição inicial para começar a desenhar o texto
    const int xPosPrm = 10;
    //  painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(95, 10, "Cupom Compra Venda");
    yPos += 20; // Avança a posição y
    painter.drawText(xPos, yPos, nomeEmpresa);
    yPos += 20;
    painter.drawText(xPos, yPos, enderecoEmpresa);
    yPos += 20;
    painter.drawText(xPos, yPos, cnpjEmpresa);
    yPos += 20;
    painter.drawText(xPos, yPos, telEmpresa);
    yPos += 20;
    painter.drawText(xPos, yPos, "Data/Hora: " + portugues2.toString(dataVenda, "dd/MM/yyyy hh:mm:ss"));
    yPos += 20;
    painter.drawText(xPos, yPos, "Cliente: " + cliente);
    yPos += 30;
    painter.drawText(xPos, yPos, "Quant:");
    int xPosProds = 45;
    xPos = xPosProds;
    painter.drawText(xPos, yPos, "Produtos vendidos:");
    int xPosValor = 202;
    xPos = xPosValor;
    painter.drawText(xPos, yPos, "ValorUn(R$):");
    yPos += 20;

    font.setPointSize(8);
    font.setBold(false);
    painter.setFont(font);
    //painter.setFont(QFont("Arial", 10));
    int lineHeight = 30; // Altura da linha
    int pageWidth = printer.pageLayout().paintRectPixels(printer.resolution()).width();

    QStringList produtos = getProdutosVendidos(idVenda);
    //QStringList descricoes = getDescricoesProdutos(produtos);
    //int index = 0;  // Índice para acessar as descrições

    for (int i = 0; i < produtos.size(); i += 3) {
        QString descricaoProduto = produtos[i];
        QString quantidadProduto = produtos[i + 1];
        QString valorProduto = produtos[i + 2];

        QTextOption textOption;
        QRect rectQuantProd(xPosPrm, yPos, xPosProds - xPosPrm, lineHeight);
        painter.drawText(rectQuantProd, quantidadProduto, textOption);

        QRect rectDesc(xPosProds, yPos, xPosValor - xPosProds - 30, lineHeight);
        textOption.setWrapMode(QTextOption::WordWrap);
        painter.drawText(rectDesc, descricaoProduto, textOption);

        QRect rectValor(xPosValor, yPos, pageWidth - xPosValor, lineHeight);
        painter.drawText(rectValor, portugues2.toString(valorProduto.toFloat(), 'f',2), textOption);

        yPos += lineHeight;
    }
    int posx = xPosPrm;
    yPos += 5;
    for(int i=0; i < pageWidth; i++){
        posx += 3;
        painter.drawText(posx,yPos, "=");
    };
    font.setBold(true);
    painter.setFont(font);
    yPos += 20;
    //    painter.drawText(Qt::AlignCenter,yPos, "Pagamento");
    xPos = 95;
    painter.drawText(xPos,yPos, "Desconto(R$): " + portugues2.toString(desconto.toFloat(),'f',2));
    yPos += 20;
    painter.drawText(xPos,yPos, "Forma Pagamento: " + forma_pagamento);
    yPos += 20;
    painter.drawText(xPos,yPos, "Valor Total Produtos(R$): " + portugues2.toString(total.toFloat(),'f',2));
    yPos += 20;
    if(forma_pagamento == "Dinheiro" ){
        painter.drawText(xPos, yPos, "Valor Recebido(R$):" + portugues2.toString(valor_recebido.toFloat(),'f',2));
        yPos += 20;
        painter.drawText(xPos,yPos, "Troco(R$):" + portugues2.toString(troco.toFloat(),'f',2));
    }else if(forma_pagamento == "Não Sei"){
    }else if(forma_pagamento == "Crédito"){
        painter.drawText(xPos, yPos, "Taxa(%):" + portugues2.toString(taxa.toFloat(),'f',2));
        yPos += 20;
        painter.drawText(xPos, yPos, "Valor Final(R$):" + portugues2.toString(valor_final.toFloat(), 'f', 2 ));

    }else if(forma_pagamento == "Débito"){
        painter.drawText(xPos, yPos, "Taxa(%):" + portugues2.toString(taxa.toFloat(),'f',2));
        yPos += 20;
        painter.drawText(xPos, yPos, "Valor Final(R$):" + portugues2.toString(valor_final.toFloat(), 'f', 2 ));

    }else if(forma_pagamento == "Pix"){
    }
    else{
        qDebug() << "forma de pagamento deu erro";
    }

    yPos += 20;
    painter.drawText(xPosPrm, yPos, "Assinatura:" );
    yPos += 50;
    painter.drawText(xPosPrm, yPos, "Obrigado Pela Compra Volte Sempre!" );
    yPos += 30;

    painter.drawText(xPosPrm,yPos, "--");


    qDebug() << printer.pageLayout().pageSize();
    painter.end();
    db2.close();

    return 1;
}
void Vendas::imprimirReciboVendaSelec(QString id){
    imprimirReciboVenda(id);
}


void Vendas::on_testebutton_clicked()
{
    //imprimirEtiquetaVenda(ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->selectionModel()->selectedIndexes().first()).toString());
    // QList<ProdutoVendido> produtos = getProdutosVendidos("85");
    // for (const ProdutoVendido &produto : produtos) {
    //     qDebug() << "ID Produto:" << produto.id_produto
    //              << "Quantidade:" << produto.quantidade
    //              << "Preço Vendido:" << produto.preco_vendido;
    // }
    // QStringList descricoes = getDescricoesProdutos(produtos);

    // // Exibir as descrições dos produtos
    // for (const QString &descricao : descricoes) {
    //     qDebug() << "Descrição do Produto:" << descricao;
    // }
}

