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
#include "customdelegate.h"
#include "delegatehora.h"
#include "delegatepago.h"
#include "delegateprecof2.h"
#include "nota/DanfeUtil.h"
#include "nota/cancelnf.h"
#include "nota/nfeacbr.h"
#include "services/recibo_service.h"

Vendas::Vendas(QWidget *parent, int idCliente) :
    QWidget(parent),
    ui(new Ui::Vendas)
{
    ui->setupUi(this);
    // configuracao modelos e views tabelas vendas e produtosvendidos

    vendaServ.listarVendas(modeloVendas2);

    modeloVendas2->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modeloVendas2->setHeaderData(1, Qt::Horizontal, tr("Valor Final"));
    modeloVendas2->setHeaderData(2, Qt::Horizontal, tr("Forma Pag"));
    modeloVendas2->setHeaderData(3, Qt::Horizontal, tr("Data e Hora"));
    modeloVendas2->setHeaderData(4, Qt::Horizontal, tr("Cliente"));
    modeloVendas2->setHeaderData(5, Qt::Horizontal, tr("Pago?"));
    modeloVendas2->setHeaderData(6, Qt::Horizontal, tr("Total"));
    modeloVendas2->setHeaderData(7, Qt::Horizontal, tr("Desconto"));
    modeloVendas2->setHeaderData(8, Qt::Horizontal, tr("Taxa"));
    modeloVendas2->setHeaderData(9, Qt::Horizontal, tr("Recebido"));
    modeloVendas2->setHeaderData(10, Qt::Horizontal, tr("Troco"));
    ui->Tview_Vendas2->setModel(modeloVendas2);

    prodVendaServ.listarProdutosVenda(modeloProdVendidos);

    ui->Tview_ProdutosVendidos->hideColumn(0);
    modeloProdVendidos->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modeloProdVendidos->setHeaderData(1, Qt::Horizontal, tr("Descrição"));
    modeloProdVendidos->setHeaderData(2, Qt::Horizontal, tr("Quantidade"));
    modeloProdVendidos->setHeaderData(3, Qt::Horizontal, tr("Preço Vendido"));

    ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);

    // esconder id produto_vendido

    ui->Tview_Vendas2->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    ui->Tview_ProdutosVendidos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    atualizarTabelas();

    // Selecionar a primeira linha das tabelas
    QModelIndex firstIndex = modeloVendas2->index(0, 0);
    ui->Tview_Vendas2->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
  // QModelIndex firstIndex2 = modeloProdVendidos->index(0, 0);
    // Obter o modelo de seleção da tabela
    QItemSelectionModel *selectionModel = ui->Tview_Vendas2->selectionModel();
    // Conectar o sinal de seleção ao slot personalizado

    // -- delegates
    DelegatePago *delegatePago = new DelegatePago(this);
 //   CustomDelegate *delegateContorno = new CustomDelegate(this);
    DelegateHora *delegateHora = new DelegateHora(this);
    DelegatePrecoF2 *delegatepreco = new DelegatePrecoF2(this);

    ui->Tview_Vendas2->setItemDelegateForColumn(1,delegatepreco);
    ui->Tview_Vendas2->setItemDelegateForColumn(5,delegatePago);
    ui->Tview_Vendas2->setItemDelegateForColumn(3,delegateHora);
    ui->Tview_ProdutosVendidos->setItemDelegateForColumn(2, delegatepreco);
    connect(selectionModel, &QItemSelectionModel::selectionChanged,this, &Vendas::handleSelectionChange);
    // ajustar tamanho colunas
    // coluna data
    ui->Tview_Vendas2->setColumnWidth(3, 150);
    ui->Tview_Vendas2->setColumnWidth(1, 90);

    // coluna cliente
      ui->Tview_Vendas2->setColumnWidth(2, 100);
    ui->Tview_Vendas2->setColumnWidth(4, 175);
    ui->Tview_Vendas2->setColumnWidth(5, 75);
    ui->Tview_Vendas2->setColumnWidth(7, 100);

    ui->Tview_Vendas2->setColumnWidth(8, 80);
    ui->Tview_Vendas2->setColumnWidth(9, 100);
    ui->Tview_ProdutosVendidos->setColumnWidth(1, 400);
    // coluna quantidade
    ui->Tview_ProdutosVendidos->setColumnWidth(2, 85);

    QPair<QDate, QDate> dateRange = vendaServ.getMinMaxData();
    qDebug() << dateRange;

    ui->DateEdt_De->setDate(dateRange.first);
    ui->DateEdt_Ate->setDate(dateRange.second);

    IDCLIENTE = idCliente;
    mostrarVendasCliente(IDCLIENTE);

    ui->Tview_Vendas2->selectionModel()->select(firstIndex, QItemSelectionModel::Select);

}

Vendas::~Vendas()
{
    delete ui;
}

void Vendas::on_Btn_InserirVenda_clicked()
{
    venda *inserirVenda = new venda;
    //inserirVenda->setWindowModality(Qt::ApplicationModal);
    connect(inserirVenda, &venda::vendaConcluida, this, &Vendas::vendaConcluidaVendas);
    connect(inserirVenda, &venda::vendaConcluida, this, &Vendas::atualizarTabelas);

    inserirVenda->show();
}

void Vendas::atualizarTabelas(){

    vendaServ.listarVendas(modeloVendas2);
    // ui->Tview_Vendas2->setModel(modeloVendas2);

    prodVendaServ.listarProdutosVendidosFromVenda(idVendaSelec.toLongLong(), modeloProdVendidos);
    // ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);

    QModelIndex firstIndex = modeloVendas2->index(0, 0);

    ui->Tview_Vendas2->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
    qDebug() << "tabelas vendas atualizadas;";
}

void Vendas::handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected) {
    // Este slot é chamado sempre que a seleção na tabela muda
    Q_UNUSED(deselected);

    QModelIndex selectedIndex = selected.indexes().first();
    QVariant idVariant = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 0));
    qlonglong productId = idVariant.toLongLong();

    prodVendaServ.listarProdutosVendidosFromVenda(productId, modeloProdVendidos);
    ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
    // ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
    QVariant idVariant2 = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 2));
    idVendaSelec = idVariant.toString();

    QString FormaPagSelec = idVariant2.toString();

    qDebug() << "Registro(s) selecionado(s):" << productId;
    if(FormaPagSelec == "Prazo"){
        ui->Btn_AbrirPag->setEnabled(true);
    }else{
        ui->Btn_AbrirPag->setEnabled(false);
    }
}

void Vendas::LabelLucro(QString de, QString ate){
    auto resumo = vendaServ.calcularResumo(de,ate,
        ui->cb_BuscaVendasPrazo->isChecked(),
        IDCLIENTE
        );

    ui->Lbl_Total->setText(portugues.toString(resumo.total, 'f', 2));
    ui->Lbl_Lucro->setText(portugues.toString(resumo.lucro, 'f', 2));
    ui->Lbl_Quantidade->setText(QString::number(resumo.quantidade));
}

void Vendas::deletarVenda(bool cancelarNf){
    if(!ui->Tview_Vendas2->currentIndex().isValid()){
        QMessageBox::warning(this,"Erro","Selecione uma venda antes de tentar deletar!");
        return;
    }

    QItemSelectionModel *selectionModel = ui->Tview_Vendas2->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();

    QString idVenda = ui->Tview_Vendas2->model()->data(
                                                    ui->Tview_Vendas2->model()->index(selectedIndex.row(),
                                                                                      0)).toString();

    float valorVendaFloat = ui->Tview_Vendas2->model()->data(
                                                          ui->Tview_Vendas2->model()->index(selectedIndex.row(), 1)).toFloat();
    QString valorVenda = portugues.toString(valorVendaFloat, 'f', 2);

    // confirmação
    QMessageBox::StandardButton resposta = QMessageBox::question(
        nullptr,
        "Confirmação",
        "Tem certeza que deseja excluir a venda:\n\n"
        "ID: " + idVenda + "\n"
                        "Valor total: " + valorVenda,
        QMessageBox::Yes | QMessageBox::No
        );

    if (resposta != QMessageBox::Yes){
        return;
    }
    // Cancelamento da NF

    if(vendaServ.vendaPossuiNota(idVenda.toLongLong()) && cancelarNf == true){
        auto respostaNf = QMessageBox::question(
            nullptr,
            "Confirmação",
            "Deseja cancelar a Nota Fiscal referente a essa venda?",
            QMessageBox::Yes | QMessageBox::No
            );

        cancelarNf = (respostaNf == QMessageBox::Yes);
    }

    auto result = vendaServ.deletarVendaRegraNegocio(idVenda.toLongLong(), cancelarNf);

    if(!result.ok){
        QMessageBox::warning(this, "Erro", result.msg);
        return;
    }

    emit vendaDeletada();
    atualizarTabelas();

}

void Vendas::on_Btn_DeletarVenda_clicked()
{
    deletarVenda(true);

}

void Vendas::on_DateEdt_De_dateChanged(const QDate &date)
{
    // precisa adicionar um dia para ele contar o dia todo
     ate = ui->DateEdt_Ate->date().addDays(1).toString("yyyy-MM-dd");
     de = date.toString("yyyy-MM-dd");
    qDebug() << de;
    qDebug() << ate;
    filtrarData(de, ate);
}

void Vendas::on_DateEdt_Ate_dateChanged(const QDate &date)
{
     de = ui->DateEdt_De->date().toString("yyyy-MM-dd");
    // precisa adicionar um dia para ele contar o dia todo
     ate = date.addDays(1).toString("yyyy-MM-dd");
    qDebug() << de;
    qDebug() << ate;
    filtrarData(de, ate);
}

void Vendas::filtrarData(QString de1, QString ate1){
    LabelLucro(de1, ate1);

    VendasUtil::VendasFormaPagamento formaPag = VendasUtil::VendasFormaPagamento::Nenhuma;
    if(ui->cb_BuscaVendasPrazo->isChecked()){
        formaPag = VendasUtil::VendasFormaPagamento::Prazo;
    }
    vendaServ.listarVendasDeAteFormaPag(modeloVendas2, de1, ate1, formaPag);
    ui->Tview_Vendas2->selectionModel()->select(QModelIndex(modeloVendas2->index(0, 0)), QItemSelectionModel::Select);

}

void Vendas::devolverProdutoVenda(QString id_venda, QString id_prod_vend)
{
    // devolver produto vendido, retornando valor e recalculando valores da venda
    bool produtoUnico = prodVendaServ.temApenasUmProduto(id_venda.toLongLong());
    qDebug() << "produtoUnico: " + QString::number(produtoUnico);

    if (produtoUnico){
        deletarVenda(false);
    }
    else{
        auto result = vendaServ.devolverProdutoRegraNegocio(id_prod_vend.toLongLong(), id_venda.toLongLong());
        if(!result.ok){
            QMessageBox::warning(this, "Erro", result.msg);
        }
    }
}

void Vendas::AtualizarTabelasSinal(){
    qDebug() << "Teste Atualizar Tabelas deletar, add valor emitido";
    filtrarData(de,ate);
}

void Vendas::actionAbrirPagamentosVenda(QString id_venda){
    EntradasVendasPrazo *pagamentosVenda = new EntradasVendasPrazo(this, id_venda);
    pagamentosVenda->setWindowModality(Qt::ApplicationModal);
    connect(pagamentosVenda, &EntradasVendasPrazo::entradaConcluida,
            this, &Vendas::AtualizarTabelasSinal);
    connect(pagamentosVenda, &EntradasVendasPrazo::entradaConcluida,
            this, &Vendas::pagamentosConcluidos);
    pagamentosVenda->show();
}

void Vendas::on_Tview_Vendas2_customContextMenuRequested(const QPoint &pos)
{
    if(!ui->Tview_Vendas2->currentIndex().isValid())
        return;
    QModelIndexList selectedRows = ui->Tview_Vendas2->selectionModel()->selectedRows();
    QString cellValue, formaPag;
    if (!selectedRows.isEmpty()) {
        // Pega o primeiro índice selecionado (caso múltiplas linhas possam ser selecionadas)
        QModelIndex selectedIndex = selectedRows.first();

        // Obtém o índice da célula na coluna 0 da linha selecionada
        QModelIndex columnIndex = ui->Tview_Vendas2->model()->index(selectedIndex.row(), 0);
        QModelIndex columnIndex2 = ui->Tview_Vendas2->model()->index(selectedIndex.row(), 2);

        // Obtém o valor da célula como uma QString
        cellValue = ui->Tview_Vendas2->model()->data(columnIndex).toString();
        formaPag = ui->Tview_Vendas2->model()->data(columnIndex2).toString();

    } else {
        QMessageBox::warning(this, "Aviso", "Nenhuma linha selecionada.");
    }

    QMenu menu(this);

    actionMenuDeletarVenda = new QAction();
    actionMenuDeletarVenda->setText("Deletar Venda");
    connect(actionMenuDeletarVenda,SIGNAL(triggered(bool)),this,SLOT(on_Btn_DeletarVenda_clicked()));

    actionImprimirRecibo = new QAction();
    actionImprimirRecibo->setText("Imprimir Recibo da Venda");
    //actionImprimirRecibo->setIcon(janelaPrincipal->iconImpressora);
    QObject::connect(actionImprimirRecibo, &QAction::triggered, [&]() {
        imprimirReciboVenda(cellValue.toLongLong());
    });

    actionAbrirPagamentos = new QAction;
    actionAbrirPagamentos->setText("Abrir Pagamentos");
    QObject::connect(actionAbrirPagamentos, &QAction::triggered, [&]() {
        actionAbrirPagamentosVenda(cellValue);
    });
    if(formaPag == "Prazo"){
        actionAbrirPagamentos->setEnabled(true);
    }else{
        actionAbrirPagamentos->setEnabled(false);
    }
    actionMenuAbrirDanfe = new QAction;
    actionMenuAbrirDanfe->setText("Abrir DANFE");
    connect(actionMenuAbrirDanfe, &QAction::triggered, [&]() {
        abrirDanfeXml(cellValue);
    });

    menu.addAction(actionMenuDeletarVenda);

    menu.addAction(actionImprimirRecibo);
    menu.addAction(actionAbrirPagamentos);
    menu.addAction(actionMenuAbrirDanfe);

    menu.exec(ui->Tview_Vendas2->viewport()->mapToGlobal(pos));
}

bool Vendas::imprimirReciboVenda(qlonglong idvenda){
    Recibo_service reciboServ;
    reciboServ.imprimirReciboVenda(idvenda);
    return true;
}
void Vendas::imprimirReciboVendaSelec(QString id){
    imprimirReciboVenda(id.toLongLong());
}

void Vendas::on_cb_BuscaVendasPrazo_stateChanged(int arg1)

{
    filtrarData(de, ate);
}

void Vendas::on_Btn_AbrirPag_clicked()
{
    actionAbrirPagamentosVenda(idVendaSelec);
}

void Vendas::on_Tview_ProdutosVendidos_customContextMenuRequested(const QPoint &pos)
{
    QModelIndexList selectedRows = ui->Tview_ProdutosVendidos->selectionModel()->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Nenhuma linha selecionada.");
        return;
    }

    QMenu menu(this);
    actionMenuDevolverProd = new QAction("Devolver Produto(s)", this);

    QObject::connect(actionMenuDevolverProd, &QAction::triggered, [=]() {

        QMessageBox::StandardButton resposta = QMessageBox::question(
            this,
            "Confirmação",
            QString("Tem certeza que deseja devolver %1 produto(s) selecionado(s)?")
                .arg(selectedRows.size()),
            QMessageBox::Yes | QMessageBox::No
            );

        if (resposta == QMessageBox::Yes) {

            QList<ProdutoVendidoDTO> listaProdsVendidos;
            QList<QString> listaIdProdsVendidos;

            for (const QModelIndex &index : selectedRows) {

                QModelIndex idRegistroVendaIndex = ui->Tview_ProdutosVendidos->model()->index(index.row(), 0);
                qlonglong idRegistroVenda = ui->Tview_ProdutosVendidos->model()->data(idRegistroVendaIndex).toLongLong();

                ProdutoVendidoDTO prodVend = prodVendaServ.getProdutoVendido(idRegistroVenda);
                listaProdsVendidos.append(prodVend);
                listaIdProdsVendidos.append(QString::number(idRegistroVenda));
            }

            bool temNota = false;
            temNota = notaServ.temNotaNormal(idVendaSelec.toLongLong());
            qDebug() << "tem nota?: " << temNota;
            qDebug() << "Qtd produtos devolução:" << listaProdsVendidos.size();
            qDebug() << "preco vendido ultimo produto vendido selec" << listaProdsVendidos.last().precoVendido;


            if (temNota) {
                auto result = fiscalServ.enviarNfeDevolucaoPadrao(idVendaSelec.toLongLong(),
                                                                  listaProdsVendidos);
                QMessageBox::information(this, "Aviso", result.msg);
                if(!result.ok){
                    qDebug() << result.msg;
                    return;
                }
            }

            // devolução normal do produto
            for (int i = 0; i < listaProdsVendidos.size(); i++) {

                devolverProdutoVenda(idVendaSelec, listaIdProdsVendidos[i]);
            }

            atualizarTabelas();
        }
    });

    menu.addAction(actionMenuDevolverProd);
    menu.exec(ui->Tview_ProdutosVendidos->viewport()->mapToGlobal(pos));
}

void Vendas::mostrarVendasCliente(int idCliente) {
    if (idCliente == 0) {
        qDebug() << "ID do cliente inválido.";
        return;
    }

    if (!db.isOpen() && !db.open()) {
        qDebug() << "Erro ao abrir banco de dados em mostrarVendasCliente: " << db.lastError().text();
        return;
    }
    //qDebug() << "IdCliente: " + QString::number(idCliente);
    QSqlQuery query;
    query.prepare("SELECT nome FROM clientes WHERE id = :idcliente");
    query.bindValue(":idcliente", idCliente);
    QString nomeCliente;
    query.exec();
    while(query.next()){
        nomeCliente = query.value(0).toString();
    }

    // Passa diretamente a string SQL para o modelo
    modeloVendas2->setQuery("SELECT id, valor_final, forma_pagamento, data_hora, "
                            "cliente, esta_pago, total, desconto, taxa, valor_recebido, "
                            "troco FROM vendas2 WHERE id_cliente = " + QString::number(idCliente) +
                                " ORDER BY id DESC", db);

    if (modeloVendas2->lastError().isValid()) {
        qDebug() << "Erro ao carregar modelo de vendas: " << modeloVendas2->lastError().text();
    }
    db.close();
    ui->Btn_InserirVenda->setDisabled(true);
    ui->Btn_InserirVenda->setVisible(false);
    ui->Lbl_ClienteHeader->setText(nomeCliente);

    filtrarData(de,ate);
}

void Vendas::abrirDanfeXml(QString id_Venda){
    QSharedPointer<DanfeUtil> danfe(new DanfeUtil(this));
    if(danfe->abrirDanfe(id_Venda.toInt())){
        qDebug() << "abrindo danfe IDVenda:" << id_Venda;
    }else{
        QMessageBox::warning(this, "Aviso", "Não foi possivel gerar DANFE com essa venda");

    }

}

void Vendas::on_cb_BuscaVendasPrazo_checkStateChanged(const Qt::CheckState &arg1)
{

}

