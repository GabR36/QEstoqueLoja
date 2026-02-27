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
                                                    ui->Tview_Vendas2->model()->index(selectedIndex.row(), 0)).toString();

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
    cancelarNf = false;

    if(vendaServ.vendaPossuiNota(idVenda.toLongLong())){
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

    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. devolver produto.";
    }
    QSqlQuery query;

    // verificar se é o unico produto para deletar a venda
    query.prepare("SELECT COUNT() FROM produtos_vendidos WHERE id_venda = :valor1");
    query.bindValue(":valor1", id_venda);
    bool produtoUnico = false;
    if (query.exec()) {
        while (query.next()) {
            produtoUnico = query.value(0).toInt() == 1;
        }
    }
    qDebug() << "produtoUnico: " + QString::number(produtoUnico);

    if (produtoUnico){
        deletarVenda(false);
    }
    else{

        // obter id do produto vendido, quantidade e preço vendido para usar depois
        query.prepare("SELECT id_produto, quantidade, preco_vendido FROM produtos_vendidos "
                      "WHERE id = :valor1");
        query.bindValue(":valor1", id_prod_vend);
        QString id_produto, qntd, preco_vend;
        if (query.exec()) {
            while (query.next()) {
                id_produto = query.value(0).toString();
                qntd = query.value(1).toString();
                preco_vend = query.value(2).toString();
            }
        }
        qDebug() << "id_produto: " + id_produto;
        qDebug() << "qntd: " + qntd;
        qDebug() << "preco_vend: " + preco_vend;

        devolverProduto(id_prod_vend, id_produto, qntd);

        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. devolver produto.";
        }

        // obter total, taxa, desconto e recebido antigos
        query.prepare("SELECT total, taxa, desconto, valor_recebido FROM vendas2 WHERE id = :valor1");
        query.bindValue(":valor1", id_venda);
        float total = 0;
        float taxa = 0;
        float desconto = 0;
        float recebido = 0;
        if (query.exec()) {
            while (query.next()) {
                total = query.value(0).toFloat();
                taxa = 1 + (query.value(1).toFloat()/100);
                desconto = query.value(2).toFloat();
                recebido = query.value(3).toFloat();
            }
        }
        else {
            qDebug() << "Algo deu errado devolver produto! query";
        }
        qDebug() << "total: " + QString::number(total);
        qDebug() << "taxa: " + QString::number(taxa);
        qDebug() << "desconto: " + QString::number(desconto);
        qDebug() << "recebido: " + QString::number(recebido);

        // mudar o registro da venda para retirar o valor do produto devolvido
        query.prepare("UPDATE vendas2 SET total = :valor1, troco = :valor2, valor_final = :valor3 "
                      "WHERE id = :valor4");
        float totalSub = qntd.toFloat() * preco_vend.toFloat();
        float totalNovo = total - totalSub;
        float valorFinalNovo = (totalNovo - desconto)*taxa;
        query.bindValue(":valor1", QString::number(totalNovo));
        query.bindValue(":valor2", QString::number(recebido - valorFinalNovo));
        query.bindValue(":valor3", QString::number(valorFinalNovo));
        query.bindValue(":valor4", id_venda);
        query.exec();
    }

    db.close();
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
{    if(!ui->Tview_Vendas2->currentIndex().isValid())
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

        // Mostra o valor em uma mensagem

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
    return 1;
}
void Vendas::imprimirReciboVendaSelec(QString id){
    imprimirReciboVenda(id.toLongLong());
}

void Vendas::on_cb_BuscaVendasPrazo_stateChanged(int arg1)

{

    filtrarData(de, ate);
}
void Vendas::on_testebutton_clicked(){
    //nao tirar
}


void Vendas::on_Btn_AbrirPag_clicked()
{


    actionAbrirPagamentosVenda(idVendaSelec);
}

QString Vendas::salvarDevolucaoNf(QString retornoEnvio, qlonglong idnf, NfeACBR *devolNfe) {
    if (retornoEnvio.isEmpty()) {
        return "Erro: Nenhum retorno do ACBr";
    }

    QStringList linhas = retornoEnvio.split('\n', Qt::SkipEmptyParts);
    QString cStat, xMotivo, msg, nProt;

    // Processa todas as linhas do retorno do ACBr
    for (const QString &linha : linhas) {
        QString linhaTrim = linha.trimmed(); // Remove espaços e quebras de linha
        if (linhaTrim.startsWith("CStat="))
            cStat = linhaTrim.section('=', 1).trimmed();
        else if (linhaTrim.startsWith("XMotivo="))
            xMotivo = linhaTrim.section('=', 1).trimmed();
        else if (linhaTrim.startsWith("Msg="))
            msg = linhaTrim.section('=', 1).trimmed();
        else if (linhaTrim.startsWith("NProt=") || linhaTrim.startsWith("nProt="))
            nProt = linhaTrim.section('=', 1).trimmed();
    }

    qDebug() << "Retorno ACBr: cStat=" << cStat << " xMotivo=" << xMotivo << " nProt=" << nProt;

    // Confirma se a nota foi autorizada
    if (cStat == "100" || cStat == "150") { // 150 é contingência autorizada
        if (!db.open()) {
            qDebug() << "Erro ao abrir banco de dados ao salvar nota de devolução";
            return "Erro: não foi possível abrir o banco de dados";
        }

        QSqlQuery query;
        QString dataFormatada = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        query.prepare("INSERT INTO notas_fiscais(cstat, nnf, serie, modelo, tp_amb, xml_path, valor_total,"
                      "atualizado_em, id_venda, cnpjemit, chnfe, nprot, cuf, finalidade, saida, id_nf_ref, dhemi) "
                      "VALUES(:cstat, :nnf, :serie, :modelo, :tpamb, :xml_path, :valortotal, :atualizadoem,"
                      ":id_venda, :cnpjemit, :chnfe, :nprot, :cuf, :finalidade, :saida, :id_nf_ref, :dhemi)");

        query.bindValue(":cstat", cStat);
        query.bindValue(":nnf", devolNfe->getNNF());
        query.bindValue(":serie", devolNfe->getSerie());
        query.bindValue(":modelo", "55");
        query.bindValue(":tpamb", devolNfe->getTpAmb());
        query.bindValue(":xml_path", devolNfe->getXmlPath());
        query.bindValue(":valortotal", QString::number(devolNfe->getVNF()));
        query.bindValue(":atualizadoem", dataFormatada);
        query.bindValue(":id_venda", idVendaSelec);
        query.bindValue(":cnpjemit", devolNfe->getCnpjEmit());
        query.bindValue(":chnfe", devolNfe->getChaveNf());
        query.bindValue(":nprot", nProt);
        query.bindValue(":cuf", devolNfe->getCuf());
        query.bindValue(":finalidade", "DEVOLUCAO");
        query.bindValue(":saida", "0");
        query.bindValue(":id_nf_ref", QString::number(idnf));
        query.bindValue(":dhemi", devolNfe->getDhEmiConvertida());

        if (!query.exec()) {
            qDebug() << "Erro ao inserir nota fiscal de devolução:" << query.lastError().text();
            return QString("Erro ao salvar nota no banco: %1").arg(query.lastError().text());
        }

        return QString("Nota de Devolução Autorizada e Salva!\n cStat:%1 \n motivo:%2 \n protocolo:%3")
            .arg(cStat, xMotivo.isEmpty() ? msg : xMotivo, nProt);

    } else {
        // Nota rejeitada
        return QString("Erro ao enviar Nota de Devolução.\n cStat:%1 \n motivo:%2 \n protocolo:%3")
            .arg(cStat, xMotivo.isEmpty() ? msg : xMotivo, nProt);
    }
}



void Vendas::on_Tview_ProdutosVendidos_customContextMenuRequested(const QPoint &pos)
{
    QModelIndexList selectedRows = ui->Tview_ProdutosVendidos->selectionModel()->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Nenhuma linha selecionada.");
        return;
    }

    QLocale usa(QLocale::English, QLocale::UnitedStates);   // <<< NOVO

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

            QList<QList<QVariant>> produtosVendidosList;
            QSqlQuery query;
            QList<QString> listaIdProdutosVendidos;

            for (const QModelIndex &index : selectedRows) {

                QModelIndex idRegistroVendaIndex = ui->Tview_ProdutosVendidos->model()->index(index.row(), 0);
                QModelIndex descIndex            = ui->Tview_ProdutosVendidos->model()->index(index.row(), 1);
                QModelIndex qntdIndex            = ui->Tview_ProdutosVendidos->model()->index(index.row(), 2);
                QModelIndex precoIndex           = ui->Tview_ProdutosVendidos->model()->index(index.row(), 3);

                QString idRegistroVenda = ui->Tview_ProdutosVendidos->model()->data(idRegistroVendaIndex).toString();
                listaIdProdutosVendidos.append(idRegistroVenda);

                // ------------------------------
                // QUANTIDADE (formato USA)
                // ------------------------------
                QString qtdStrOriginal = ui->Tview_ProdutosVendidos->model()->data(qntdIndex).toString();

                // limpa e converte
                QString qtdClean = qtdStrOriginal;
                qtdClean.replace(',', '.'); // caso venha em BR
                double qtdNum = qtdClean.toDouble();

                // sempre formatar com ponto e 4 casas
                QString quantidadeUSA = usa.toString(qtdNum, 'f', 4);

                // ------------------------------
                // PREÇO UNITÁRIO (formato USA)
                // ------------------------------
                QString precoStrOriginal = ui->Tview_ProdutosVendidos->model()->data(precoIndex).toString();

                QString precoClean = precoStrOriginal;
                precoClean.replace(',', '.');
                double precoNum = precoClean.toDouble();

                // formato US com 2 casas
                QString precoUSA = usa.toString(precoNum, 'f', 2);

                // ----------------------------------------------------
                // BUSCA O ID REAL DO PRODUTO
                // ----------------------------------------------------
                QString idProduto;
                {
                    db.open();
                    query.prepare("SELECT id_produto FROM produtos_vendidos WHERE id = :id_registro");
                    query.bindValue(":id_registro", idRegistroVenda);
                    if (query.exec() && query.next()) {
                        idProduto = query.value(0).toString();
                    } else {
                        qWarning() << "Erro ao buscar id_produto para produtos_vendidos.id =" << idRegistroVenda
                                   << ":" << query.lastError().text();
                        continue;
                    }
                }

                // ----------------------------------------------------
                // MONTA LISTA  agora em FORMATO USA
                // ----------------------------------------------------
                QList<QVariant> produtoInfo;

                produtoInfo << idProduto       // ID real do produto
                            << quantidadeUSA   // quantidade US
                            << ui->Tview_ProdutosVendidos->model()->data(descIndex)
                            << precoUSA;       // preço US

                produtosVendidosList.append(produtoInfo);
            }


            // Envio normal para NFe...
            if (!db.open()) {
                qDebug() << "banco nao abriu para pegar chavenf";
            }

            query.prepare("SELECT chnfe FROM notas_fiscais WHERE id_venda = :idvenda AND finalidade = 'NORMAL'");
            query.bindValue(":idvenda", idVendaSelec);

            QString chnfe;
            bool temNota = false;

            if (query.exec() && query.next()) {
                chnfe = query.value(0).toString();
                temNota = true;
            }

            if (temNota) {
                NfeACBR *nfe = new NfeACBR(this, false, true);
                nfe->setNNF(nfe->getProximoNNF());
                nfe->setNfRef(chnfe);
                nfe->setProdutosVendidos(produtosVendidosList, false);

                QString retorno = nfe->gerarEnviar();
                QString msg = salvarDevolucaoNf(retorno, notaServ.getIdFromIdVenda(idVendaSelec.toLongLong()), nfe);
                QMessageBox::information(this, "Aviso", msg);
            }

            // devolução normal do produto
            for (int i = 0; i < listaIdProdutosVendidos.size(); i++) {
                devolverProdutoVenda(idVendaSelec, listaIdProdutosVendidos[i]);
            }

            qDebug() << "Produtos enviados p/ NFe (USA):";
            for (const auto &linha : produtosVendidosList) {
                qDebug() << linha;
            }

            atualizarTabelas();
        }
    });

    menu.addAction(actionMenuDevolverProd);
    menu.exec(ui->Tview_ProdutosVendidos->viewport()->mapToGlobal(pos));
}



void Vendas::devolverProduto(QString id_prod_vend, QString id_produto, QString qntd)
{
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. devolver produto.";
    }
    QSqlQuery query2;

    // deletar registro do produto devolvido
    query2.prepare("DELETE FROM produtos_vendidos WHERE id = :valor1");
    query2.bindValue(":valor1", id_prod_vend);
    if (query2.exec()) {
        qDebug() << "query bem-sucedido!";
    } else {
        qDebug() << "Erro no query: ";
    }

    // devolver produto ao estoque
    query2.prepare("UPDATE produtos SET quantidade = quantidade + :valor1 WHERE id = :valor2");
    query2.bindValue(":valor1", qntd);
    query2.bindValue(":valor2", id_produto);
    query2.exec();
    emit devolvidoProduto();

    // db.close();
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

