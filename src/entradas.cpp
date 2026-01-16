#include "entradas.h"
#include "ui_entradas.h"
#include "nota/acbrmanager.h"
#include "configuracao.h"
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QFile>
#include <QDomDocument>
#include <QRegularExpression>
#include <QDateTime>
#include "util/manifestadordfe.h"
#include "subclass/leditdialog.h"
#include <qmenu.h>
#include "inserirproduto.h"
#include "configuracao.h"
#include <QFile>
#include <QDomDocument>
#include <QDebug>
#include "util/nfxmlutil.h"
#include "util/dbutil.h"
#include "mergeprodutos.h"
#include "delegatepago.h"
#include "util/mailmanager.h"
#include <QDir>
#include <QSqlError>
#include <QDebug>
#include "delegatehora.h"

Entradas::Entradas(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Entradas)
{
    ui->setupUi(this);

    empresaValues = Configuracao::get_All_Empresa_Values();
    db = QSqlDatabase::database();
    financeiroValues = Configuracao::get_All_Financeiro_Values();
    produtoValues = Configuracao::get_All_Produto_Values();

    carregarTabela();
    connect(ui->Tview_Entradas->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Entradas::on_EntradaSelecionada);

    nfe = new NfeACBR(this, true, true);

    //delegate SIM VERDE NAO VERMELHO para coluna 'adicionado' da tabela produtos
    DelegatePago *delegateSimNao = new DelegatePago(this);
    ui->Tview_ProdutosNota->setItemDelegateForColumn(5, delegateSimNao);
    DelegateHora *delegateData = new DelegateHora(this);
    ui->Tview_Entradas->setItemDelegateForColumn(2, delegateData);

    QDate hoje = QDate::currentDate();
    QDate primeiroDia = QDate(hoje.year(), hoje.month(), 1).addMonths(-1);
    QDate ultimoDia = QDate(hoje.year(), hoje.month(), 1).addMonths(1).addDays(-1);

    ui->DateEdt_De->setDate(primeiroDia);
    ui->DateEdt_Ate->setDate(ultimoDia);

    ui->Tview_Entradas->selectRow(0);
}

Entradas::~Entradas()
{
    delete ui;
}

QString Entradas::converterDataSefaz(const QString &data){
    if (data.isEmpty())
        return "";

    // Formato SEFAZ: 2025-11-29T16:17:31-03:00
    QDateTime dt = QDateTime::fromString(data, Qt::ISODate);

    // Se falhar, tente sem timezone
    if (!dt.isValid())
        dt = QDateTime::fromString(data, "yyyy-MM-dd'T'hh:mm:ss");

    if (!dt.isValid())
        return data; // retorna como veio

    return dt.toString("yyyy-MM-dd HH:mm:ss");
}

void Entradas::salvarRegistroDFe(
    const QString &nome_emitente,
    const QString &data_emissao,
    const QString &vnf,
    const QString &nsu,
    const QString &tipo,
    const QString &chave,
    const QString &cnpj,
    const QString &situacao,
    const QString &xml,
    const QString &data_recebimento)
{
    QSqlQuery query(db);

    query.prepare(R"(
        INSERT OR IGNORE INTO dfe_registros
        (nome_emitente, data_emissao, vnf, nsu, tipo, chave, cnpj, situacao, xml_path, data_recebimento)
        VALUES (:nome_emitente, :data_emissao, :vnf, :nsu, :tipo, :chave, :cnpj, :situacao, :xml_path, :data_recebimento)
    )");

    query.bindValue(":nome_emitente", nome_emitente);
    query.bindValue(":data_emissao", data_emissao);
    query.bindValue(":vnf", vnf);
    query.bindValue(":nsu", nsu);
    query.bindValue(":tipo", tipo);
    query.bindValue(":chave", chave);
    query.bindValue(":cnpj", cnpj);
    query.bindValue(":situacao", situacao);

    query.bindValue(":xml_path", xml);
    query.bindValue(":data_recebimento", data_recebimento);

    if (!query.exec()) {
        qDebug() << "Erro ao inserir registro:" << query.lastError().text();
    }
}

void Entradas::on_Btn_ConsultarDF_clicked()
{
    ManifestadorDFe *manifestdfe = new ManifestadorDFe(this);
    if(manifestdfe->possoConsultar()){

        manifestdfe->consultaAlternada();
        QMessageBox::information(this, "Resposta", "Consulta realizada com sucesso.");
        atualizarTabela();
    }else{
        QMessageBox::warning(this, "Aviso", "Não faz uma hora que a última consulta "
                                               "foi realizada, por favor espere.");
    }
    ui->Tview_Entradas->selectRow(0);
}

void Entradas::atualizarTabela(QString whereSql)
{
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Erro ao abrir banco atualizarTabela()";
            return;
        }
    }
    
    QString sql = R"(
        SELECT
            c.nome AS emitente,
            n.valor_total AS valor,
            n.dhemi AS emissao,
            n.cnpjemit AS cnpj,
            n.modelo AS modelo,
            n.chnfe AS chave,
            n.cstat AS cstat,
            n.id AS id_nf
        FROM notas_fiscais n
        LEFT JOIN clientes c
            ON c.id = n.id_emissorcliente
        WHERE n.finalidade = 'ENTRADA EXTERNA'
          AND n.cstat IN (100, 150)
    )";

    if (!whereSql.isEmpty()) {
        sql += " AND " + whereSql;
    }

    sql += " ORDER BY n.dhemi DESC";

    qDebug() << sql;

    modelEntradas->setQuery(sql);
    ui->Tview_Entradas->selectRow(0);
}


void Entradas::carregarTabela()
{
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Erro ao abrir banco carregarTabela()";
        }
    }
    modelEntradas = new QSqlQueryModel(this);

    modelEntradas->setQuery(R"(
        SELECT
            c.nome AS emitente,
            n.valor_total AS valor,
            n.dhemi AS emissao,
            n.cnpjemit AS cnpj,
            n.modelo AS modelo,
            n.chnfe AS chave,
            n.cstat AS cstat,
            n.id AS id_nf
        FROM notas_fiscais n
        LEFT JOIN clientes c
            ON c.id = n.id_emissorcliente
        WHERE n.finalidade = 'ENTRADA EXTERNA'
          AND n.cstat = 100
        ORDER BY n.dhemi DESC
    )");

    if (modelEntradas->lastError().isValid()) {
        qDebug() << "Erro ao carregar tabela:" << modelEntradas->lastError();
    }

    modelEntradas->setHeaderData(0, Qt::Horizontal, "Emitente");
    modelEntradas->setHeaderData(1, Qt::Horizontal, "Valor NF");
    modelEntradas->setHeaderData(2, Qt::Horizontal, "Emissão");
    modelEntradas->setHeaderData(3, Qt::Horizontal, "CNPJ");
    modelEntradas->setHeaderData(4, Qt::Horizontal, "Modelo");
    modelEntradas->setHeaderData(5, Qt::Horizontal, "Chave");
    modelEntradas->setHeaderData(6, Qt::Horizontal, "CStat");

    ui->Tview_Entradas->setModel(modelEntradas);

    ui->Tview_Entradas->resizeColumnsToContents();
    ui->Tview_Entradas->horizontalHeader()->setStretchLastSection(true);
    ui->Tview_Entradas->setModel(modelEntradas);
    ui->Tview_Entradas->setColumnHidden(7, true); // Oculta id_nf
    db.close();
}

void Entradas::on_EntradaSelecionada(const QModelIndex &current, const QModelIndex &previous)
{
    if (!current.isValid())
        return;

    // Coluna onde está o id_nf (a 7ª do SELECT)
    int row = current.row();
    QModelIndex idIndex = current.model()->index(row, 7);

    id_nf_selec = current.model()->data(idIndex).toLongLong();

    carregarProdutosDaNota(id_nf_selec);
}

void Entradas::carregarProdutosDaNota(qlonglong id_nf)
{
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Erro ao abrir banco carregarProdutosDaNota()";
        }
    }
    QSqlQueryModel *model = new QSqlQueryModel(this);

    model->setQuery(QString(R"(
        SELECT
            id as id,
            nitem AS Item,
            quantidade AS Quantidade,
            descricao AS Descrição,
            preco AS Preço,
            adicionado AS Adicionado,
            status AS Status,
            un_comercial AS Unidade,
            ncm AS NCM,
            cfop AS CFOP,
            csosn AS CSOSN,
            codigo_barras AS CódigoBarras
        FROM produtos_nota
        WHERE id_nf = %1
        ORDER BY nitem
    )").arg(id_nf));

    if (model->lastError().isValid()) {
        qDebug() << "Erro ao carregar produtos:" << model->lastError();
    }

    ui->Tview_ProdutosNota->setModel(model);
    ui->Tview_ProdutosNota->resizeColumnsToContents();
    ui->Tview_ProdutosNota->horizontalHeader()->setStretchLastSection(true);
    ui->Tview_ProdutosNota->setModel(model);
    ui->Tview_ProdutosNota->setColumnHidden(0, true);  // esconde o id
    db.close();
}

bool Entradas::existeCodBarras(QString codigo)
{
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Banco nao abriu em existeCodBarras()";
            return false;
        }
    }

    QSqlQuery query(db);
    query.prepare("SELECT 1 FROM produtos WHERE codigo_barras = :cod LIMIT 1");
    query.bindValue(":cod", codigo);

    if (!query.exec()) {
        qDebug() << "Erro ao executar SELECT em existeCodBarras():" << query.lastError().text();
        db.close();
        return false;
    }

    // Se encontrou alguma linha, existe
    if (query.next()) {
        db.close();
        return true;
    }
    db.close();
    return false;
}



void Entradas::on_Tview_ProdutosNota_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->Tview_ProdutosNota->indexAt(pos);
    if (!index.isValid())
        return;

    // Todas as linhas selecionadas
    QModelIndexList selecionadas = ui->Tview_ProdutosNota->selectionModel()->selectedRows();

    if (selecionadas.isEmpty())
        return;



    QList<qlonglong> idsSelecionados;
    bool jaDevolvido = false;

    for (const QModelIndex &linha : selecionadas) {
        int id = ui->Tview_ProdutosNota->model()
        ->data(ui->Tview_ProdutosNota->model()->index(linha.row(), 0))
            .toInt();
        idsSelecionados.append(id);

        QString status = ui->Tview_ProdutosNota->model()
                             ->data(ui->Tview_ProdutosNota->model()->index(linha.row(), 6))
                             .toString();

        if (status.trimmed().toUpper() == "DEVOLVIDO") {
            jaDevolvido = true;
        }
    }

    // Apenas para debug
    qDebug() << "IDs selecionados:" << idsSelecionados;
    qDebug() << "ID do primeiro produto selecionado:" << idsSelecionados.first();

    QMenu menu(this);
    QAction *adicionar = menu.addAction("Adicionar ao Estoque");
    QAction *devolucao = menu.addAction("Emitir Devolução");

    if (jaDevolvido) {
        devolucao->setEnabled(false);
    }

    QAction *selecionada = menu.exec(ui->Tview_ProdutosNota->viewport()->mapToGlobal(pos));
    if (!selecionada)
        return;

    if (selecionada == adicionar) {
        LeditDialog *barcodePage = new LeditDialog(this);
        barcodePage->setLabelText("Digite ou escaneie o código do produto selecionado:");
        barcodePage->show();

        if (barcodePage->exec() == QDialog::Accepted) {
            QString codigoEscaneado = barcodePage->getLineEditText();

            if (existeCodBarras(codigoEscaneado) && !codigoEscaneado.isEmpty()) {
                QMessageBox::warning(this, "Aviso", "Já existe um produto com esse código cadastrado.");

                addProdComCodBarras(QString::number(idsSelecionados.first()), codigoEscaneado);
            } else {
                addProdSemCodBarras(QString::number(idsSelecionados.first()), codigoEscaneado);
            }
        }

    } else if (selecionada == devolucao) {
        QMessageBox::StandardButton resposta = QMessageBox::question(
            this,
            "Confirmação",
            QString("Tem certeza que deseja emitir uma nota de devolução de "
                    "%1 produto(s) selecionado(s)?")
                .arg(idsSelecionados.size()),
            QMessageBox::Yes | QMessageBox::No
            );
        if(resposta == QMessageBox::Yes){
            devolverProdutos(idsSelecionados);
        }
    }
}

void Entradas::devolverProdutos(QList<qlonglong> &idsProduto){
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Banco nao abriu devolverProdutos()";
        }
    }
    // void NfeACBR::setCliente(bool ehPf, QString cpf, QString nome, int indiedest,
    //                          QString email, QString lgr, QString nro, QString bairro, QString cmun, QString xmun,
    //                          QString uf, QString cep, QString ie
    QSqlQuery query;
    query.prepare("SELECT id_emissorcliente, chnfe FROM notas_fiscais WHERE id = :idnota");
    query.bindValue(":idnota", id_nf_selec);
    QString idcliente, chavenfe;
    if(query.exec()){
        while(query.next()){
            idcliente = query.value(0).toString();
            chavenfe = query.value(1).toString();
        }
    }else{
        qDebug() << "QUERY nao rodou para achar o idclienteemissor em devolverproduto";
        return;
    }

    Cliente dest;

    query.prepare("SELECT eh_pf, cpf, nome, indIEDest, email, endereco, numero_end, bairro, "
                  "cMun, xMun, uf, cep, ie FROM clientes WHERE id = :idcliente");
    query.bindValue(":idcliente", idcliente);

    if(query.exec()){
        while(query.next()){
            dest.nome = query.value("nome").toString();
            dest.ehpf = query.value("eh_pf").toBool();
            dest.cpfcnpj = query.value("cpf").toString();
            dest.indiedest = query.value("indIEDest").toInt();
            dest.email = query.value("email").toString();
            dest.endereco = query.value("endereco").toString();
            dest.numero_end = query.value("numero_end").toString();
            dest.bairro = query.value("bairro").toString();
            dest.cmun = query.value("cMun").toString();
            dest.xmun = query.value("xMun").toString();
            dest.uf = query.value("uf").toString();
            dest.cep = query.value("cep").toString();
            dest.ie = query.value("ie").toString();
        }
    }else{
        qDebug() << "QUERY nao rodou para achar as infos do idclienteemissor em devolverproduto";
        return;
    }

    nfe->setNNF(nfe->getProximoNNF());
    nfe->setNfRef(chavenfe);
    nfe->setProdutosNota(idsProduto);
    nfe->setCliente(dest.ehpf, dest.cpfcnpj, dest.nome, dest.indiedest, dest.email,
    dest.endereco, dest.numero_end, dest.bairro, dest.cmun, dest.xmun, dest.uf, dest.cep,
    dest.ie);
    QString retorno = nfe->gerarEnviar();
    QString msg = salvarDevolucaoNf(retorno, QString::number(id_nf_selec), nfe, idsProduto);
    QMessageBox::information(this, "Aviso", msg);

    if(cstatRetornado == "100" || cstatRetornado == "150" && nfe->getTpAmb() == "1"){
        enviarEmailNFe(dest.nome, dest.email, nfe->getXmlPath(), nfe->getPdfDanfe(), dest.cpfcnpj);
    }

    db.close();

}

void Entradas::enviarEmailNFe(QString nomeCliente, QString emailCliente,
                                    QString xmlPath, std::string pdfDanfe, QString cnpj){

    try {

        QDateTime data = QDateTime::currentDateTime();

        auto mail = MailManager::instance().mail();
        QByteArray pdfBytes = QByteArray::fromBase64(
            QByteArray::fromStdString(pdfDanfe)
            );
        QString pdfPath =
            QDir::tempPath() + "/DANFE_" +
            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") +
            ".pdf";

        QFile pdfFile(pdfPath);
        if (!pdfFile.open(QIODevice::WriteOnly)) {
            qDebug() << "Erro ao criar PDF DANFE";
            return;
        }
        pdfFile.write(pdfBytes);
        pdfFile.close();

        QString corpo;
        QString nomeEmpresa = empresaValues.value("nome_empresa");
        QString dataFormatada = portugues.toString(
            data,
            "dddd, dd 'de' MMMM 'de' yyyy 'às' HH:mm"
            );

        corpo = "Olá " + nomeCliente + "\n\n"
            "Foi emitido uma Nota Fiscal de Devolução da " + nomeEmpresa + " para o CNPJ:" +
               cnpj +
                "\n\nem anexo, você encontrará os arquivos referentes à "
                                "Nota Fiscal de " +
                dataFormatada + ".\n\n"
                                "Cordialmente,\n\n" +
                nomeEmpresa;
        mail->Limpar();
        mail->LimparAnexos();
        mail->AddCorpoAlternativo(corpo.toStdString());
        mail->SetAssunto("Nota Fiscal Eletrônica de " + empresaValues.value("nome_empresa").toStdString());
        mail->AddDestinatario(emailCliente.toStdString());
        mail->AddAnexo(xmlPath.toStdString(), "XML NFe", 0);
        mail->AddAnexo(pdfPath.toStdString(), "DANFE (PDF)", 0);

        mail->Enviar();
        qDebug() << "email enviado NFE";
    }
    catch (const std::exception& e) {
        qDebug() << "email não enviado NFE";
    }
}

QString Entradas::salvarDevolucaoNf(QString retornoEnvio, QString idnf, NfeACBR *devolNfe,
                                    QList<qlonglong> &idsProduto) {


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
    cstatRetornado = cStat;
    qDebug() << "Retorno ACBr: cStat=" << cStat << " xMotivo=" << xMotivo << " nProt=" << nProt;

    // Confirma se a nota foi autorizada
    if (cStat == "100" || cStat == "150") { // 150 é contingência autorizada
        if(!db.isOpen()){
            if(!db.open()){
                qDebug() << "Erro ao abrir banco em salvarDevolucaoNf";
                return "Erro: não foi possível abrir o banco de dados";

            }
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
        query.bindValue(":id_venda", "");
        query.bindValue(":cnpjemit", devolNfe->getCnpjEmit());
        query.bindValue(":chnfe", devolNfe->getChaveNf());
        query.bindValue(":nprot", nProt);
        query.bindValue(":cuf", devolNfe->getCuf());
        query.bindValue(":finalidade", "DEVOLUCAO");
        query.bindValue(":saida", "1");
        query.bindValue(":id_nf_ref", idnf);
        query.bindValue(":dhemi", devolNfe->getDhEmiConvertida());

        if (!query.exec()) {
            qDebug() << "Erro ao inserir nota fiscal de devolução:" << query.lastError().text();
            return QString("Erro ao salvar nota no banco: %1").arg(query.lastError().text());
        }else{
            lastInsertedIDNfDevol = query.lastInsertId().toString();
            atualizarProdutoNotaAoDevolver(lastInsertedIDNfDevol, idsProduto);
            return QString("Nota de Devolução Autorizada e Salva!\n cStat:%1 \n Motivo:%2 \n")
                .arg(cStat, xMotivo.isEmpty() ? msg : xMotivo, nProt);
        }



    } else {
        // Nota rejeitada
        return QString("Erro ao enviar Nota de Devolução.\n cStat:%1 \n motivo:%2 \n protocolo:%3")
            .arg(cStat, xMotivo.isEmpty() ? msg : xMotivo, nProt);
    }
    db.close();
}

void Entradas::atualizarProdutoNotaAoDevolver(QString idNfDevol, QList<qlonglong> &idsProduto) {
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Banco nao abriu em atualizarProdutoNotaAoDevolver()";
            return;
        }
    }

    QSqlQuery query;

    // Prepara apenas uma vez para não recriar o statement repetidamente
    query.prepare("UPDATE produtos_nota "
                  "SET status = 'DEVOLVIDO', id_nfDevol = :idnfdevol "
                  "WHERE id = :idprod");

    for (qlonglong idProd : idsProduto) {
        query.bindValue(":idnfdevol", idNfDevol);
        query.bindValue(":idprod", idProd);

        if (!query.exec()) {
            qDebug() << "Erro ao atualizar produto" << idProd
                     << ":" << query.lastError().text();
        }
    }

    qDebug() << "Atualização de produtos concluída.";
}

void Entradas::addProdSemCodBarras(QString idProd, QString codBarras){
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Banco nao abriu em addProdSemCodBarras()";
        }
    }
    QString quant;
    QString desc;
    QString preco;
    QString cod_barras;
    QString un_comercial;
    QString ncm;
    QString csosn;
    QString pis;
    QSqlQuery query;
    QString xml_path;
    int nitem;
    query.prepare("SELECT quantidade, descricao, preco, codigo_barras, un_comercial, ncm, csosn, pis, nitem, xml_path "
                  "FROM produtos_nota INNER JOIN notas_fiscais ON id_nf = notas_fiscais.id WHERE produtos_nota.id = :id");
    query.bindValue(":id", idProd);
    if(query.exec()){
        while(query.next()){
            quant = query.value("quantidade").toString();
            desc = query.value("descricao").toString();
            preco = query.value("preco").toString();
            cod_barras = query.value("codigo_barras").toString();
            un_comercial = query.value("un_comercial").toString();
            ncm = query.value("ncm").toString();
            csosn = query.value("csosn").toString();
            pis = query.value("pis").toString();
            nitem = query.value("nitem").toInt();
            xml_path = query.value("xml_path").toString();

        }
    }else{
        qDebug() << "query addProdSemCodBarras nao rodou";
    }
    quant = portugues.toString(quant.toFloat());

    NfXmlUtil *nfutil = new NfXmlUtil(this);
    CustoItem custoxml = nfutil->calcularCustoItemSN(xml_path, nitem);

    qDebug() << "custo fornecedor taxas incluidas:" << custoxml.custoUnitario;
    qDebug() << "Preço fornecedor " << custoxml.precoUnitarioNota;
    double coreValue = custoxml.custoUnitario;
    preco = portugues.toString(coreValue, 'f', 2);

    QString porcent_lucro = portugues.toString(financeiroValues.value("porcent_lucro").toFloat());
    QString pisCofins = produtoValues.value("pis_padrao");
    QString csosnPadrao = produtoValues.value("csosn_padrao");

    InserirProduto *addProd = new InserirProduto();
    addProd->preencherCamposProduto(
        quant, desc, "", codBarras, 1, un_comercial, preco, porcent_lucro, ncm, "", csosnPadrao,
        pisCofins
    );
    addProd->show();
    connect(addProd, &InserirProduto::produtoInserido, this,
            &Entradas::produtoAdicionado);

    connect(addProd, &InserirProduto::produtoInserido, this,
            [=]() {
                atualizarProdutoNotaAdicionado(idProd);
            });
    connect(addProd, &InserirProduto::produtoInserido, this,
            [=]() {
                carregarProdutosDaNota(id_nf_selec);
            });

    db.close();
}

void Entradas::atualizarProdutoNotaAdicionado(QString idProd){
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Banco nao abriu atualizarProdutoNotaAdicionado";
        }
    }

    QSqlQuery query;
    query.prepare("UPDATE produtos_nota SET adicionado = 1 WHERE id = :id");
    query.bindValue(":id", idProd);

    if(query.exec()){
    }else{
        qDebug() << "query nao rodou atualizarProdutoNotaAdicionado. ";
        return;
    }
    qDebug() << "query update produto nota adicionado = 1 ok";
}


void Entradas::addProdComCodBarras(QString idProd, QString codBarras){
    qDebug() << idProd << codBarras;
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Banco nao abriu em addProdComCodBarras()";
        }
    }

    QSqlQuery query;

    query.prepare("SELECT * FROM produtos WHERE produtos.codigo_barras = :codBarras");
    query.bindValue(":codBarras", codBarras);
    if (!query.exec()) {
        qDebug() << "erro query addprodcombarras()";
    }

    QVector<QVariantMap> registros = DBUtil::extrairResultados(query);

    if (registros.isEmpty()) {
        qDebug() << "Produto não encontrado";
        return;
    }

    QVariantMap produto = registros.first();

    QSqlQuery query2;

    query2.prepare("SELECT produtos_nota.id, quantidade, descricao, preco, codigo_barras, "
                   "un_comercial, ncm, aliquota_imposto, csosn, nitem, pis, xml_path "
                   "FROM produtos_nota "
                   "INNER JOIN notas_fiscais ON produtos_nota.id_nf = notas_fiscais.id "
                   "WHERE produtos_nota.id = :idProd");
    query2.bindValue(":idProd", idProd);
    if (!query2.exec()) {
        qDebug() << "erro query addprodcombarras()2";
    }

    QVector<QVariantMap> registros2 = DBUtil::extrairResultados(query2);

    if (registros2.isEmpty()) {
        qDebug() << "ProdutoNota não encontrado";
        return;
    }

    QVariantMap produtoNota = registros2.first();

    NfXmlUtil *nfutil = new NfXmlUtil(this);
    CustoItem custoxml = nfutil->calcularCustoItemSN(produtoNota["xml_path"].toString(), produtoNota["nitem"].toInt());

    produtoNota["preco"] = custoxml.custoUnitario;

    // comparar os campos do produtoNota e produto e pegar a diferença
    QVariantMap resultado;

    resultado["quantidade"] = produto["quantidade"].toDouble() + produtoNota["quantidade"].toFloat();

    if (produto["descricao"].toString() != produtoNota["descricao"].toString()) {
        if (produto["descricao"].toString() == "") {
            resultado["descricao"] = produtoNota["descricao"].toString();
        }
        else {
            resultado["descricao"] = produto["descricao"].toString();
        }
    }

    if (produto["preco_fornecedor"].toDouble() != produtoNota["preco"].toDouble()) {
        float porcent_lucro = financeiroValues.value("porcent_lucro").toDouble();
        resultado["preco"] = produtoNota["preco"].toDouble() * (porcent_lucro/100 + 1);
        resultado["porcent_lucro"] = porcent_lucro;
        resultado["preco_fornecedor"] = produtoNota["preco"].toDouble();
    }

    if (produto["un_comercial"].toString() != produtoNota["un_comercial"].toString()) {
        if (produto["un_comercial"].toString() == "") {
            resultado["un_comercial"] = produtoNota["un_comercial"].toString();
        }
        else {
            resultado["un_comercial"] = produto["un_comercial"].toString();
        }
    }

    if (produto["ncm"].toString() != produtoNota["ncm"].toString()) {
        if (produto["ncm"].toString() == "" || produto["ncm"] == "00000000") {
            resultado["ncm"] = produtoNota["ncm"].toString();
        }
        else {
            resultado["ncm"] = produto["ncm"].toString();
        }
    }

    if (produto["aliquota_imposto"].toDouble() != produtoNota["aliquota_imposto"].toDouble()) {
        if (produto["aliquota_imposto"].toDouble() == 0) {
            resultado["aliquota_imposto"] = produtoNota["aliquota_imposto"].toDouble();
        }
        else {
            resultado["aliquota_imposto"] = produto["aliquota_imposto"].toDouble();
        }
    }

    db.close();

    qDebug() << "produto: " << produto;

    qDebug() << "produtoNota: " << produtoNota;

    qDebug() << "resultado: " << resultado;

    MergeProdutos *janelaMerge = new MergeProdutos(produto, produtoNota, resultado);
    janelaMerge->show();

    connect(janelaMerge, &MergeProdutos::produtoAtualizado, this,
            [=]() {
                atualizarProdutoNotaAdicionado(idProd);
            });
    connect(janelaMerge, &MergeProdutos::produtoAtualizado, this,
            &Entradas::produtoAdicionado);
    connect(janelaMerge, &MergeProdutos::produtoAtualizado, this,
            [=]() {
                carregarProdutosDaNota(id_nf_selec);
            });

}

void Entradas::on_DateEdt_De_userDateChanged(const QDate &date)
{
    QString ate = ui->DateEdt_Ate->date().addDays(1).toString("yyyy-MM-dd");
    QString de = date.toString("yyyy-MM-dd");

    QString where = "n.dhemi BETWEEN '" + de + "' AND '" +
        ate + "'";

    atualizarTabela(where);
}


void Entradas::on_DateEdt_Ate_userDateChanged(const QDate &date)
{
    QString ate = date.addDays(1).toString("yyyy-MM-dd");
    QString de = ui->DateEdt_De->date().toString("yyyy-MM-dd");

    QString where = "n.dhemi BETWEEN '" + de + "' AND '" +
        ate + "'";

    atualizarTabela(where);
}

