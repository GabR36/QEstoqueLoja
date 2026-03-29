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
#include "util/ibptutil.h"

Entradas::Entradas(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Entradas)
{
    ui->setupUi(this);
    Config_service *confServ = new Config_service(this);
    configDTO = confServ->carregarTudo();

    carregarTabela();
    connect(ui->Tview_Entradas->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Entradas::on_EntradaSelecionada);

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

void Entradas::on_Btn_ConsultarDF_clicked()
{
    ManifestadorDFe *manifestdfe = new ManifestadorDFe(this);
    if(dfeServ.possoConsultar()){

        manifestdfe->consultaAlternada();
        QMessageBox::information(this, "Resposta", "Consulta realizada com sucesso.");
        atualizarTabela();
    }else{
        QMessageBox::warning(this, "Aviso", "Não faz uma hora que a última consulta "
                                               "foi realizada, por favor espere.");
    }
    ui->Tview_Entradas->selectRow(0);
}

void Entradas::atualizarTabela(const QString &de, const QString &ate)
{
    notaServ.listarEntradas(modelEntradas, de, ate);
    ui->Tview_Entradas->selectRow(0);
}

void Entradas::carregarTabela()
{
    modelEntradas = new QSqlQueryModel(this);
    modelProdutosNota = new QSqlQueryModel(this);
    atualizarTabela("", "");

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
    ui->Tview_Entradas->setColumnHidden(7, true); // Oculta id_nf

    ui->Tview_ProdutosNota->setModel(modelProdutosNota);
    ui->Tview_ProdutosNota->horizontalHeader()->setStretchLastSection(true);
    ui->Tview_ProdutosNota->setColumnHidden(0, true); // Oculta id
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
    prodNotaServ.listarPorNota(modelProdutosNota, id_nf);
    ui->Tview_ProdutosNota->resizeColumnsToContents();
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

    // adiciona produtos selecionados a lista
    QList<ProdutoNotaDTO> prodSelecionados;
    bool jaDevolvido = false;

    for (const QModelIndex &linha : selecionadas) {
        qlonglong id = ui->Tview_ProdutosNota->model()
        ->data(ui->Tview_ProdutosNota->model()->index(linha.row(), 0))
            .toLongLong();

        ProdutoNotaDTO prod = prodNotaServ.getProdutoNota(id);
        if(!prod.descricao.isEmpty()){
            prodSelecionados.append(prod);
        }
        if(prod.status == "DEVOLVIDO"){
            jaDevolvido = true;
        }
    }

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

            if (prodServ.codigoBarrasExiste(codigoEscaneado) && !codigoEscaneado.isEmpty()) {
                QMessageBox::warning(this, "Aviso", "Já existe um produto com esse código cadastrado.");

                addProdComCodBarras(QString::number(prodSelecionados.first().id), codigoEscaneado);
            } else {
                addProdSemCodBarras(QString::number(prodSelecionados.first().id), codigoEscaneado);
            }
        }

    } else if (selecionada == devolucao) {
        QMessageBox::StandardButton resposta = QMessageBox::question(
            this,
            "Confirmação",
            QString("Tem certeza que deseja emitir uma nota de devolução de "
                    "%1 produto(s) selecionado(s)?")
                .arg(prodSelecionados.size()),
            QMessageBox::Yes | QMessageBox::No
            );
        if(resposta == QMessageBox::Yes){
            devolverProdutos(prodSelecionados);
        }
    }
}

void Entradas::devolverProdutos(QList<ProdutoNotaDTO> &produtosNota){
    NotaFiscalDTO notaRef = notaServ.getNotaById(id_nf_selec);
    if(notaRef.chNfe.isEmpty()){
        QMessageBox::warning(this, "Erro", "Nota fiscal de referência não encontrada.");
        return;
    }

    ClienteDTO cliente = clienteServ.getClienteByID(notaRef.idEmissorCliente);

    auto resultado = fiscalEmitterServ.enviarNfeDevolucaoEntrada(id_nf_selec, produtosNota, cliente);

    QMessageBox::information(this, "Aviso", resultado.msg);

    if(resultado.ok){
        carregarProdutosDaNota(id_nf_selec);
    }
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
        QString nomeEmpresa = configDTO.nomeEmpresa;
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
        mail->SetAssunto("Nota Fiscal Eletrônica de " + configDTO.nomeEmpresa.toStdString());
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

void Entradas::addProdSemCodBarras(QString idProd, QString codBarras){
    qlonglong id = idProd.toLongLong();

    ProdutoNotaDTO prod = prodNotaServ.getProdutoNota(id);
    QString xml_path = prodNotaServ.getXmlPathPorId(id);

    NfXmlUtil *nfutil = new NfXmlUtil(this);
    CustoItem custoxml = nfutil->calcularCustoItemSN(xml_path, prod.nitem);

    qDebug() << "custo fornecedor taxas incluidas:" << custoxml.custoUnitario;
    qDebug() << "Preço fornecedor " << custoxml.precoUnitarioNota;

    ProdutoDTO novoProd;
    novoProd.quantidade      = prod.quantidade;
    novoProd.descricao       = prod.descricao;
    novoProd.codigoBarras    = codBarras;
    novoProd.nf              = true;
    novoProd.uCom            = prod.uCom;
    novoProd.precoFornecedor = custoxml.custoUnitario;
    novoProd.percentLucro    = configDTO.porcentLucroFinanceiro;
    novoProd.ncm             = prod.ncm;
    novoProd.csosn           = configDTO.csosnPadraoProduto;
    novoProd.pis             = configDTO.pisPadraoProduto;

    InserirProduto *addProd = new InserirProduto();
    addProd->preencherCamposProduto(novoProd);
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
}

void Entradas::atualizarProdutoNotaAdicionado(QString idProd){
    if(!prodNotaServ.marcarComoAdicionado(idProd.toLongLong())){
        qDebug() << "atualizarProdutoNotaAdicionado falhou para id:" << idProd;
        return;
    }
    qDebug() << "query update produto nota adicionado = 1 ok";
}


void Entradas::addProdComCodBarras(QString idProd, QString codBarras){
    qDebug() << idProd << codBarras;

    QVariantMap produto = prodServ.getProdutoPorCodBarrasMap(codBarras);
    if (produto.isEmpty()) {
        qDebug() << "Produto não encontrado";
        return;
    }

    QVariantMap produtoNota = prodNotaServ.getProdutoNotaComXmlPath(idProd.toLongLong());
    if (produtoNota.isEmpty()) {
        qDebug() << "ProdutoNota não encontrado";
        return;
    }

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
        double porcent_lucro = configDTO.porcentLucroFinanceiro;
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

    qDebug() << "teste antes: " << resultado["ncm"];

    if (resultado.contains("ncm")) {
        IbptUtil *util = new IbptUtil(this);
        resultado["aliquota_imposto"] = util->get_Aliquota_From_Csv(resultado["ncm"].toString());
    }

    qDebug() << "produto: " << produto;

    qDebug() << "produtoNota: " << produtoNota;

    qDebug() << "resultado: " << resultado;

    qDebug() << "teste: " << (produto["ncm"].toString() != produtoNota["ncm"].toString());

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
    QString de = date.toString("yyyy-MM-dd");
    QString ate = ui->DateEdt_Ate->date().addDays(1).toString("yyyy-MM-dd");
    atualizarTabela(de, ate);
}


void Entradas::on_DateEdt_Ate_userDateChanged(const QDate &date)
{
    QString de = ui->DateEdt_De->date().toString("yyyy-MM-dd");
    QString ate = date.addDays(1).toString("yyyy-MM-dd");
    atualizarTabela(de, ate);
}

