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


Vendas::Vendas(QWidget *parent, int idCliente) :
    QWidget(parent),
    ui(new Ui::Vendas)
{
    ui->setupUi(this);

    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao venda.";
    }
    // configuracao modelos e views tabelas vendas e produtosvendidos
    modeloVendas2->setQuery("SELECT id, valor_final,forma_pagamento, data_hora, cliente, esta_pago, total, desconto, taxa, valor_recebido, troco FROM vendas2 ORDER BY id DESC");
    ui->Tview_Vendas2->setModel(modeloVendas2);
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


    modeloProdVendidos->setQuery("SELECT * FROM produtos_vendidos");
    ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
    // esconder id produto_vendido
    ui->Tview_ProdutosVendidos->hideColumn(0);
    modeloProdVendidos->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modeloProdVendidos->setHeaderData(1, Qt::Horizontal, tr("Descrição"));
    modeloProdVendidos->setHeaderData(2, Qt::Horizontal, tr("Quantidade"));
    modeloProdVendidos->setHeaderData(3, Qt::Horizontal, tr("Preço Vendido"));
    db.close();
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
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao venda.";
    }
    modeloVendas2->setQuery("SELECT id, valor_final,forma_pagamento, data_hora, cliente,"
                            " esta_pago, total, desconto, taxa, valor_recebido, troco FROM "
                            "vendas2 ORDER BY id DESC");

    modeloProdVendidos->setQuery("SELECT * FROM produtos_vendidos");


    db.close();


    QModelIndex firstIndex = modeloVendas2->index(0, 0);



    ui->Tview_Vendas2->selectionModel()->select(firstIndex, QItemSelectionModel::Select);

    // Definir a primeira linha como a linha atual
    //selectionModel->setCurrentIndex(topLeft, QItemSelectionModel::NoUpdate);
    qDebug() << "tabelas vendas atualizadas;";
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
        QString quantidade = query.value(1).toString();
        QString precoVendido = query.value(2).toString();

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
    modeloProdVendidos->setQuery("SELECT produtos_vendidos.id, produtos.descricao, "
                                 "produtos_vendidos.quantidade, produtos_vendidos.preco_vendido "
                                 "FROM produtos_vendidos JOIN produtos "
                                 "ON produtos_vendidos.id_produto = produtos.id "
                                 "WHERE id_venda = " + productId);
    // ui->Tview_ProdutosVendidos->setModel(modeloProdVendidos);
    QVariant idVariant2 = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 2));
    idVendaSelec = idVariant.toString();
    QString FormaPagSelec = idVariant2.toString();
    if(FormaPagSelec == "Prazo"){
        ui->Btn_AbrirPag->setEnabled(true);
    }else{
            ui->Btn_AbrirPag->setEnabled(false);
        }
    db.close();



}

void Vendas::LabelLucro(QString whereQueryData, QString whereQueryPrazo, QString whereQueryCliente){
    // colocar valores nos labels de lucro etc
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. labelLucro";
    }
    QString whereQuery = whereQueryData + whereQueryPrazo + whereQueryCliente;
    QSqlQuery query;
    float total = 0;
    int quantidadeVendas = 0;
    // SUM(total - desconto)
    if (query.exec("SELECT id, total, desconto, forma_pagamento FROM vendas2 " + whereQuery)) {
        while (query.next()) {
            if (query.value(3).toString() == "Prazo"){
                // caso seja a prazo, somar as entradas da venda ao inves to total
                QSqlQuery queryEntradas;
                queryEntradas.exec("SELECT total, desconto FROM entradas_vendas " + whereQueryData + " AND id_venda = " + query.value(0).toString());
                while(queryEntradas.next()){
                    qDebug() << "debug cont: " + queryEntradas.value(0).toString() + " " + queryEntradas.value(1).toString();
                    total += queryEntradas.value(0).toFloat() - queryEntradas.value(1).toFloat();
                }
            }
            else{
                total += query.value(1).toFloat() - query.value(2).toFloat();
            }
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

int Vendas::getNfId(int id_venda) {
    QSqlQuery query;

    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados.";
        return -1;
    }

    query.prepare("SELECT id FROM notas_fiscais WHERE id_venda = :idvenda");
    query.bindValue(":idvenda", id_venda);

    if (!query.exec()) {
        qDebug() << "Erro ao executar query:" << query.lastError().text();
        return -1;
    }

    if (query.next()) { // Move para o primeiro registro (se houver)
        return query.value(0).toInt(); // Retorna o primeiro campo (id)
    } else {
        qDebug() << "Nenhum registro encontrado para id_venda =" << id_venda;
        return -1;
    }
}

QString Vendas::salvarEvento(QString retorno, int id_nf)
{
    if(!db.open()){
        qDebug() << "Erro ao abrir banco de dados (salvarEvento)";
        return "Erro: Banco de dados não aberto";
    }

    QString tipo_evento, cstat, justificativa, codigo, xml_path, nprot;
    int id_lote = 1;

    // Usa QRegularExpression para extrair dados do texto
    QRegularExpression rx_idlote("idLote=(\\d+)");
    QRegularExpression rx_tpEvento("tpEvento=(\\d+)");
    QRegularExpression rx_xEvento("xEvento=([^\n]+)");
    QRegularExpression rx_nProt("nProt=(\\d+)");
    QRegularExpression rx_xJust("xJust=([^<\n]+)");
    QRegularExpression rx_arquivo("arquivo=([^\n]+)");
    QRegularExpression rx_cstat("CStat=(\\d+)");
    QRegularExpression rx_dh("dhRegEvento=(\\d{2}/\\d{2}/\\d{4} \\d{2}:\\d{2}:\\d{2})");
    QRegularExpression rx_xMotivo("XMotivo=([^\n]+)");


    auto match = rx_idlote.match(retorno);
    if (match.hasMatch()) id_lote = match.captured(1).toInt();

    // match = rx_cstat.match(retorno);
    // if (match.hasMatch()) cstat = match.captured(1).trimmed();

    match = rx_tpEvento.match(retorno);
    if (match.hasMatch()) codigo = match.captured(1).trimmed();

    match = rx_xEvento.match(retorno);
    if (match.hasMatch()) tipo_evento = match.captured(1).trimmed();

    match = rx_nProt.match(retorno);
    if (match.hasMatch()) nprot = match.captured(1).trimmed();

    match = rx_xJust.match(retorno);
    if (match.hasMatch()) justificativa = match.captured(1).trimmed();

    match = rx_arquivo.match(retorno);
    if (match.hasMatch()) xml_path = match.captured(1).trimmed();

    match = rx_dh.match(retorno);

    QDateTime dataIngles;
    if (match.hasMatch()) {
        QString dataStr = match.captured(1);
        dataIngles = QDateTime::fromString(dataStr, "dd/MM/yyyy HH:mm:ss");
    } else {
        // Se não achar, usa a data atual
        dataIngles = QDateTime::currentDateTime();
    }

    // match = rx_xMotivo.match(retorno);
    // if (match.hasMatch()) {
    //     xMotivo = match.captured(1).trimmed();
    // }

    QRegularExpressionMatchIterator it = rx_cstat.globalMatch(retorno);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        cstat = m.captured(1).trimmed(); // sobrescreve até o último
    }

    QString xMotivo;
    it = rx_xMotivo.globalMatch(retorno);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        xMotivo = m.captured(1).trimmed(); // sobrescreve até o último
    }
    // Debug opcional
    qDebug() << "Evento:" << tipo_evento << "| Prot:" << nprot << "| CStat:" << cstat;

    if(cstat.trimmed() == "135"){
        QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");
        // Grava no banco
        QSqlQuery query;
        query.prepare(R"(
            INSERT INTO eventos_fiscais
            (tipo_evento, id_lote, cstat, justificativa, codigo, xml_path, nprot, id_nf, atualizado_em)
            VALUES (:tipo_evento, :id_lote, :cstat, :justificativa, :codigo, :xml_path, :nprot, :id_nf,
     :atualizado_em)
        )");


        query.bindValue(":tipo_evento", tipo_evento);
        query.bindValue(":id_lote", id_lote);
        query.bindValue(":cstat", cstat);
        query.bindValue(":justificativa", justificativa);
        query.bindValue(":codigo", codigo);
        query.bindValue(":xml_path", xml_path);
        query.bindValue(":nprot", nprot);
        query.bindValue(":id_nf", id_nf);
        query.bindValue(":atualizado_em", dataFormatada);

        if(!query.exec()){
            qDebug() << "Erro ao inserir evento fiscal:" << query.lastError().text();
            return "Erro ao salvar evento no banco";
        }

        query.prepare("UPDATE notas_fiscais SET cstat = :novocstat WHERE id = :idnf ");
        query.bindValue(":novocstat", cstat);
        query.bindValue(":idnf", id_nf);
        if(!query.exec()){
            qDebug() << "Erro ao atualizar cstat da nota cancelada:" << query.lastError().text();
        }

        return "Evento cancelamento salvo com sucesso!";
    }else{
        QString msgErro = QString("Erro ao processar evento. \nCStat: %1").arg(cstat);
        if (!xMotivo.isEmpty())
            msgErro += QString("\nMotivo: %1").arg(xMotivo);

        qDebug() << msgErro;
        return msgErro;
    }
}
void Vendas::deletarVenda(bool cancelarNf){
    if(ui->Tview_Vendas2->currentIndex().isValid()){
        // obter id selecionado
        QItemSelectionModel *selectionModel = ui->Tview_Vendas2->selectionModel();
        QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
        QVariant idVariant = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 0));
        QVariant valorVariant = ui->Tview_Vendas2->model()->data(ui->Tview_Vendas2->model()->index(selectedIndex.row(), 1));
        QString productId = idVariant.toString();
        QString productValor = portugues.toString(valorVariant.toFloat(), 'f', 2);

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
            if(cancelarNf){
                //se existe nf com o id da venda
                if(getNfId(productId.toInt()) != -1){
                    qDebug() << "getnfid" << getNfId(productId.toInt());
                    QMessageBox::StandardButton respostaNf;
                    respostaNf = QMessageBox::question(
                        nullptr,
                        "Confirmação",
                        "Deseja cancelar a Nota Fiscal referente a essa venda?:\n\n",
                        QMessageBox::Yes | QMessageBox::No
                        );
                    if(respostaNf == QMessageBox::Yes){
                        int idNf = getNfId(productId.toInt());
                        cancelNf *evento = new cancelNf(this, idNf);
                        QString retorno = evento->gerarEnviar();
                        QString msg = salvarEvento(retorno, idNf);
                        QMessageBox::information(this, "Aviso", msg);
                        if (!msg.contains("sucesso", Qt::CaseInsensitive)) {
                            return;
                        }
                    }else{
                        //nao cancela a nf mas tenta deletar a venda msm assim
                    }
                }
            }


            // remover registro do banco de dados
            if(!db.open()){
                qDebug() << "erro ao abrir banco de dados. botao deletar.";
            }
            QSqlQuery query;

            // pegar os ids dos produtos e quantidades para adicionar depois
            query.prepare("SELECT id, id_produto, quantidade FROM produtos_vendidos WHERE id_venda = :valor1");
            query.bindValue(":valor1", productId);
            if (query.exec()) {
                qDebug() << "query bem-sucedido!";
            } else {
                qDebug() << "Erro no query: ";
            }
            while (query.next()){
                QString idProdVend = query.value(0).toString();
                QString idProduto = query.value(1).toString();
                QString quantProduto = query.value(2).toString();
                qDebug() << idProdVend;
                qDebug() << idProduto;
                qDebug() <<  quantProduto;
                devolverProduto(idProdVend, idProduto, quantProduto);
            }

            if(!db.open()){
                qDebug() << "erro ao abrir banco de dados. botao deletar.";
            }

            query.prepare("DELETE FROM entradas_vendas WHERE id_venda = :valor1");
            query.bindValue(":valor1", productId);
            if (query.exec()) {
                qDebug() << "query delete entrada bem-sucedido!";
            } else {
                qDebug() << "Erro no delete entrada ";
            }

            // deletar a venda
            query.prepare("DELETE FROM vendas2 WHERE id = :valor1");
            query.bindValue(":valor1", productId);
            if (query.exec()) {
                qDebug() << "Delete bem-sucedido!";
            } else {
                qDebug() << "Erro no Delete: ";
            }
            emit vendaDeletada();
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
    QString whereQueryData;
    QString whereQueryPrazo;
    QString whereQueryCliente;
    if(ui->cb_BuscaVendasPrazo->isChecked()){
        whereQueryPrazo =  " AND forma_pagamento = 'Prazo'";
    }
    if(IDCLIENTE > 0){
        whereQueryCliente =  " AND id_cliente = " + QString::number(IDCLIENTE);
    }
    whereQueryData = QString("WHERE data_hora BETWEEN '%1' AND '%2'").arg(de1, ate1);
    QString whereQuery = whereQueryData + whereQueryPrazo + whereQueryCliente;

    LabelLucro(whereQueryData, whereQueryPrazo, whereQueryCliente);
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. filtrarData";
    }
    qDebug() << "wherequery2: >" + whereQuery;
    modeloVendas2->setQuery("SELECT id, valor_final,forma_pagamento, data_hora, cliente, esta_pago, total, desconto, taxa, valor_recebido, troco FROM vendas2 " + whereQuery + " ORDER BY id DESC");
    db.close();
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
        imprimirReciboVenda(cellValue);
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
    painter.drawText(xPos, yPos, "Valor(R$):");
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
        painter.drawText(rectValor, portugues2.toString(valorProduto.toFloat() * quantidadProduto.toInt(), 'f',2), textOption);

        yPos += lineHeight;
    }
    int posx = xPosPrm;
    yPos += 5;
    for(int i=0; i < pageWidth; i++){
        posx += 3;
        painter.drawText(posx,yPos, "=");
    };
    // font.setBold(true);
    // painter.setFont(font);
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
        painter.drawText(xPos,yPos, "Valor Final(R$): " + portugues2.toString(valor_final.toFloat(),'f',2));
        yPos += 20;
        painter.drawText(xPos, yPos, "Valor Recebido(R$):" + portugues2.toString(valor_recebido.toFloat(),'f',2));
        yPos += 20;
        painter.drawText(xPos,yPos, "Troco(R$):" + portugues2.toString(troco.toFloat(),'f',2));
    }else if(forma_pagamento == "Não Sei"){
        painter.drawText(xPos,yPos, "Valor Final(R$): " + portugues2.toString(valor_final.toFloat(),'f',2));
        yPos += 20;
    }else if(forma_pagamento == "Crédito"){
        painter.drawText(xPos, yPos, "Taxa(%):" + portugues2.toString(taxa.toFloat(),'f',2));
        yPos += 20;
        painter.drawText(xPos, yPos, "Valor Final(R$):" + portugues2.toString(valor_final.toFloat(), 'f', 2 ));

    }else if(forma_pagamento == "Débito"){
        painter.drawText(xPos, yPos, "Taxa(%):" + portugues2.toString(taxa.toFloat(),'f',2));
        yPos += 20;
        painter.drawText(xPos, yPos, "Valor Final(R$):" + portugues2.toString(valor_final.toFloat(), 'f', 2 ));

    }else if(forma_pagamento == "Pix"){
        painter.drawText(xPos,yPos, "Valor Final(R$): " + portugues2.toString(valor_final.toFloat(),'f',2));
        yPos += 20;
    }else if(forma_pagamento == "Prazo"){
        painter.drawText(xPos,yPos, "Valor Final(R$): " + portugues2.toString(valor_final.toFloat(),'f',2));
        yPos += 20;
        posx = 0;
        for(int i=0; i < pageWidth; i++){
            posx += 3;
            painter.drawText(posx,yPos, "=");
        };
        if(!db2.open()){
            qDebug() <<"erro ao abrir banco de dados prazom impresao";
        }
        query.prepare("SELECT total, data_hora, forma_pagamento, valor_recebido, troco, taxa, valor_final, desconto FROM entradas_vendas WHERE id_venda = :valoridvenda");
        query.bindValue(":valoridvenda", idVenda);
        int rowEntrada = 1;
        float devendoEntrada = valor_final.toFloat();
        int xPosParcela = xPosValor - 70;
        if(query.exec()){
            while(query.next()){
                QString totalEntrada = query.value("total").toString();
                QDateTime data_horaEntrada = query.value("data_hora").toDateTime();
                QString forma_pagamentoEntrada = query.value("forma_pagamento").toString();
                QString valor_recebidoEntrada = query.value("valor_recebido").toString();
                QString trocoEntrada = query.value("troco").toString();
                QString taxaEntrada = query.value("taxa").toString();
                QString valorFinalEntrada = query.value("valor_final").toString();
               // QString descontoEntrada = query.value("desconto").toString();
                xPos = 60;
                yPos += 30;
                for(int i=0; i < pageWidth; i++){
                    posx += 3;
                    painter.drawText(posx,yPos, "=");
                };
                font.setBold(true);
                font.setUnderline(true);
                painter.setFont(font);
                painter.drawText(xPos, yPos, "Parcela: " + QString::number(rowEntrada));
                yPos += 20;
                xPos=xPosPrm;
                font.setUnderline(false);
                font.setBold(false);
                painter.setFont(font);

                    if(forma_pagamentoEntrada == "Dinheiro" ){
                        font.setBold(true);
                        painter.setFont(font);
                        painter.drawText(xPos, yPos, "Descontado(R$): " + portugues2.toString(totalEntrada.toFloat(),'f',2));
                        font.setBold(false);
                        painter.setFont(font);
                        xPos = xPosParcela;
                        painter.drawText(xPos, yPos, "Data: " + portugues2.toString(data_horaEntrada, "dd/MM/yyyy hh:mm"));
                        yPos += 20;
                        xPos = xPosPrm;
                        painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);
                        xPos = xPosParcela;
                        painter.drawText(xPos, yPos, "Valor Rec.(R$): " + portugues2.toString(valor_recebidoEntrada.toFloat(),'f',2));
                        yPos += 20;
                        painter.drawText(xPos,yPos, "Troco(R$): " + portugues2.toString(trocoEntrada.toFloat(),'f',2));
                    }else if(forma_pagamentoEntrada == "Não Sei"){
                        font.setBold(true);
                        painter.setFont(font);
                        painter.drawText(xPos, yPos, "Descontado(R$): " + portugues2.toString(totalEntrada.toFloat(),'f',2));
                        font.setBold(false);
                        painter.setFont(font);
                        xPos = xPosParcela;
                        painter.drawText(xPos, yPos, "Data: " + portugues2.toString(data_horaEntrada, "dd/MM/yyyy hh:mm"));
                        yPos += 20;
                        painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);
                        yPos += 20;
                    }else if(forma_pagamentoEntrada == "Crédito"){
                        font.setBold(true);
                        painter.setFont(font);
                        painter.drawText(xPos, yPos, "Descontado(R$): " + portugues2.toString(totalEntrada.toFloat(),'f',2));
                        font.setBold(false);
                        painter.setFont(font);
                        xPos = xPosParcela;
                        painter.drawText(xPos, yPos, "Data: " + portugues2.toString(data_horaEntrada, "dd/MM/yyyy hh:mm"));
                        yPos += 20;
                        xPos = xPosPrm;
                        painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);
                        xPos = xPosParcela;
                        painter.drawText(xPos, yPos, "Taxa(%): " + portugues2.toString(taxaEntrada.toFloat(),'f',2));
                        yPos += 20;
                        painter.drawText(xPos, yPos, "Valor Final(R$): " + portugues2.toString(valorFinalEntrada.toFloat(), 'f', 2 ));

                    }else if(forma_pagamentoEntrada == "Débito"){
                        font.setBold(true);
                        painter.setFont(font);
                        painter.drawText(xPos, yPos, "Descontado(R$): " + portugues2.toString(totalEntrada.toFloat(),'f',2));
                        font.setBold(false);
                        painter.setFont(font);
                        xPos = xPosParcela;
                        painter.drawText(xPos, yPos, "Data: " + portugues2.toString(data_horaEntrada, "dd/MM/yyyy hh:mm"));
                        yPos += 20;
                        xPos = xPosPrm;
                        painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);
                        xPos = xPosParcela;
                        painter.drawText(xPos, yPos, "Taxa(%): " + portugues2.toString(taxaEntrada.toFloat(),'f',2));
                        yPos += 20;
                        painter.drawText(xPos, yPos, "Valor Final(R$): " + portugues2.toString(valorFinalEntrada.toFloat(), 'f', 2 ));

                    }else if(forma_pagamentoEntrada == "Pix"){
                        font.setBold(true);
                        painter.setFont(font);
                        painter.drawText(xPos, yPos, "Descontado(R$): " + portugues2.toString(totalEntrada.toFloat(),'f',2));
                        font.setBold(false);
                        painter.setFont(font);
                        xPos = xPosParcela;
                        painter.drawText(xPos, yPos, "Data: " + portugues2.toString(data_horaEntrada, "dd/MM/yyyy hh:mm"));
                        yPos += 20;
                        xPos = xPosPrm;
                        painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);

                    }else{
                        qDebug() << "forma de pagamentro entrada nao encontrada";
                    }
                    for(int i=0; i < pageWidth; i++){
                        posx += 3;
                        painter.drawText(posx,yPos, "=");
                    };


                devendoEntrada -= totalEntrada.toFloat();
                rowEntrada++;

            }
            font.setBold(true);
            font.setUnderline(true);
            painter.setFont(font);
            xPos = xPosParcela;
            yPos += 20;
            painter.drawText(xPos, yPos, "Devendo(R$):" + portugues2.toString(devendoEntrada,'f',2));
            yPos += 20;
            font.setBold(false);
            font.setUnderline(false);
            painter.setFont(font);


        }

    }else{
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

QString Vendas::salvarDevolucaoNf(QString retornoEnvio, int idnf, NfeACBR *devolNfe){
    if (retornoEnvio.isEmpty()) {
        return "Erro: Nenhum retorno do ACBr";
    }

    QStringList linhas = retornoEnvio.split('\n', Qt::SkipEmptyParts);
    QString cStat, xMotivo, msg, nProt;

    for (const QString &linha : linhas) {
        if (linha.startsWith("CStat="))
            cStat = linha.section('=', 1);
        else if (linha.startsWith("XMotivo="))
            xMotivo = linha.section('=', 1);
        else if (linha.startsWith("Msg="))
            msg = linha.section('=', 1);
        else if (linha.startsWith("NProt=") || linha.startsWith("nProt="))
            nProt = linha.section('=', 1);
    }
    if(cStat == "100"){

        if(!db.open()){
            qDebug() << "bd nao abriu ao salvar nf devol";
        }
        QSqlQuery query;
        QDateTime dataAtual = QDateTime::currentDateTime();
        QString dataFormatada = dataAtual.toString("yyyy-MM-dd HH:mm:ss");


        query.prepare("INSERT INTO notas_fiscais(cstat, nnf, serie, modelo, tp_amb, xml_path, valor_total,"
                      "atualizado_em, id_venda, cnpjemit, chnfe, nprot, cuf, finalidade, saida, id_nf_ref) "
                      "VALUES(:cstat, :nnf, :serie, :modelo, :tpamb, :xml_path, :valortotal, :atualizadoem,"
                      ":id_venda, :cnpjemit, :chnfe, :nprot, :cuf, :finalidade, :saida, :id_nf_ref)");
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
        if (!query.exec()) {
            qDebug() << "Erro ao inserir nota fiscal de devolução:" << query.lastError().text();
            return QString("Erro ao salvar nota no banco: %1").arg(query.lastError().text());
        }

        if (query.numRowsAffected() > 0) {
            return QString("Nota de Devolução Autorizada e Salva!\n cStat:%1 \n motivo:%2 \n protocolo:%3")
                .arg(cStat, xMotivo.isEmpty() ? msg : xMotivo, nProt);
        } else {
            return "Erro: Nota não foi salva no banco de dados";
        }
    } else {
        return QString("Erro ao Enviar Nota de Devolução.\n cStat:%1 \n motivo:%2 \n protocolo:%3")
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

                // Colunas do modelo atual (sem mexer no SELECT original)
                QModelIndex idRegistroVendaIndex = ui->Tview_ProdutosVendidos->model()->index(index.row(), 0); // produtos_vendidos.id
                QModelIndex descIndex            = ui->Tview_ProdutosVendidos->model()->index(index.row(), 1);
                QModelIndex qntdIndex            = ui->Tview_ProdutosVendidos->model()->index(index.row(), 2);
                QModelIndex precoIndex           = ui->Tview_ProdutosVendidos->model()->index(index.row(), 3);

                QString idRegistroVenda = ui->Tview_ProdutosVendidos->model()->data(idRegistroVendaIndex).toString();
                listaIdProdutosVendidos.append(idRegistroVenda);
                // Converter QUANTIDADE
                QString quantidadeOriginal = ui->Tview_ProdutosVendidos->model()->data(qntdIndex).toString();
                double quantidadeNum = 0;
                bool okQuantidade = false;

                // Remove possíveis formatações e converte para double
                QString qtdClean = quantidadeOriginal;
                qtdClean.replace(',', '.'); // Converte para formato numérico padrão

                quantidadeNum = qtdClean.toDouble(&okQuantidade);

                if (!okQuantidade) {
                    qWarning() << "Erro na conversão da quantidade:" << quantidadeOriginal;
                    continue;
                }

                // Formata com 4 casas decimais e vírgula
                QString quantidadeFormatada = portugues.toString(quantidadeNum, 'f', 4);

                // Converter PREÇO
                QString precoOriginal = ui->Tview_ProdutosVendidos->model()->data(precoIndex).toString();
                double precoNum = 0;
                bool okPreco = false;

                // Remove possíveis formatações e converte para double
                QString precoClean = precoOriginal;
                precoClean.replace(',', '.'); // Converte para formato numérico padrão

                precoNum = precoClean.toDouble(&okPreco);

                if (!okPreco) {
                    qWarning() << "Erro na conversão do preço:" << precoOriginal;
                    continue;
                }

                // Formata com 2 casas decimais e vírgula (IMPORTANTE: sempre 2 casas)
                QString precoFormatado = portugues.toString(precoNum, 'f', 2);

                //Busca o ID real do produto no banco
                QString idProduto;
                {
                    db.open();
                    query.prepare("SELECT id_produto FROM produtos_vendidos WHERE id = :id_registro");
                    query.bindValue(":id_registro", idRegistroVenda);

                    if (query.exec() && query.next()) {
                        idProduto = query.value(0).toString();
                    } else {
                        qWarning() << "Erro ao buscar id_produto para produtos_vendidos.id =" << idRegistroVenda << ":" << query.lastError().text();
                        continue; // pula este item se não encontrar
                    }
                }

                // Monta a sublista com o ID real do produto
                QList<QVariant> produtoInfo;
                produtoInfo << idProduto
                            << quantidadeFormatada
                            << ui->Tview_ProdutosVendidos->model()->data(descIndex)
                            << precoFormatado;

                produtosVendidosList.append(produtoInfo);


            }

            // aqui começa tramite nota fiscal devolução

            if(!db.open()){
                qDebug() << "banco nao abriu para pegar chavenf";
            }
            query.prepare("SELECT chnfe FROM notas_fiscais WHERE id_venda = :idvenda "
                          "AND finalidade = 'NORMAL' ");
            query.bindValue(":idvenda", idVendaSelec);
            QString chnfe;
            bool temNota = false;

            if(query.exec()){
                if(query.next()) { // move para o primeiro resultado
                    chnfe = query.value(0).toString();
                    temNota = true;
                } else {
                    qDebug() << "Nenhum resultado encontrado para id_venda =" << idVendaSelec;
                    temNota = false;

                }

            } else {
                qDebug() << "Erro ao executar query:" << query.lastError().text();
            }
            qDebug() << "CHAVE NFE: " << chnfe;

            if(temNota){
                NfeACBR *nfe = new NfeACBR(this, false, true);
                nfe->setNNF(nfe->getProximoNNF());
                nfe->setNfRef(chnfe);
                nfe->setProdutosVendidos(produtosVendidosList, false);
                QString retorno = nfe->gerarEnviar();
                QString msg = salvarDevolucaoNf(retorno, getNfId(idVendaSelec.toInt()), nfe);
                QMessageBox::information(this,"Aviso", msg);

            }
            //aqui devolve o produto banco de dados normal

            for(int i=0; i < listaIdProdutosVendidos.size(); i++){
                // Continua usando o ID do registro da venda para a devolução
                if (!listaIdProdutosVendidos[i].isEmpty()) {
                    qDebug() << "Devolvendo registro da venda ID:" << listaIdProdutosVendidos[i]
                             << "(produto real ID:" << listaIdProdutosVendidos[i] << ") da venda:" << idVendaSelec;
                    devolverProdutoVenda(idVendaSelec, listaIdProdutosVendidos[i]);
                }
            }




            // Debug opcional
            qDebug() << "Produtos vendidos (dados completos):";
            for (const QList<QVariant> &linha : produtosVendidosList) {
                qDebug() << linha;
            }


            atualizarTabelas();

        } else {
            qDebug() << "Devolução cancelada pelo usuário.";
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

    db.close();
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
