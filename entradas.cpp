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

    // Agora armazenamos xml (preferencialmente o path se houver)
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
        QMessageBox::information(this, "Resposta", "Consultado");
        carregarTabela();
    }else{
        QMessageBox::information(this, "Erro", "Espere uma hora para consultar");
    }
    ui->Tview_Entradas->selectRow(0);
}

void Entradas::carregarTabela()
{
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Erro ao abrir banco carregarTabela()";
        }
    }
    QSqlQueryModel *model = new QSqlQueryModel(this);

    model->setQuery(R"(
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

    if (model->lastError().isValid()) {
        qDebug() << "Erro ao carregar tabela:" << model->lastError();
    }

    model->setHeaderData(0, Qt::Horizontal, "Emitente");
    model->setHeaderData(1, Qt::Horizontal, "Valor NF");
    model->setHeaderData(2, Qt::Horizontal, "Emissão");
    model->setHeaderData(3, Qt::Horizontal, "CNPJ");
    model->setHeaderData(4, Qt::Horizontal, "Modelo");
    model->setHeaderData(5, Qt::Horizontal, "Chave");
    model->setHeaderData(6, Qt::Horizontal, "CStat");

    ui->Tview_Entradas->setModel(model);

    ui->Tview_Entradas->resizeColumnsToContents();
    ui->Tview_Entradas->horizontalHeader()->setStretchLastSection(true);
    ui->Tview_Entradas->setModel(model);
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

    int id_nf = current.model()->data(idIndex).toInt();

    carregarProdutosDaNota(id_nf);
}

void Entradas::carregarProdutosDaNota(int id_nf)
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
            descricao AS Descrição,
            quantidade AS Quantidade,
            preco AS Preço,
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
        return; // clicar fora da tabela não exibe menu

    // Descobrir linha selecionada (produto)
    int row = index.row();
    int id_produto_nota = ui->Tview_ProdutosNota->model()
                              ->data(ui->Tview_ProdutosNota->model()->index(row, 0))
                              .toInt();
    qDebug() << "ID do produto_nota selecionado:" << id_produto_nota;


    QMenu menu(this);
    QAction *adicionar = menu.addAction("Adicionar ao Estoque");


    QAction *selecionada = menu.exec(ui->Tview_ProdutosNota->viewport()->mapToGlobal(pos));
    if (!selecionada)
        return;

    if (selecionada == adicionar) {
        qDebug() << "Adicionar produto da linha:" << row;
        LeditDialog *barcodePage = new LeditDialog(this);
        barcodePage->setLabelText("Digite ou escaneie o código do produto\nselecionado:");
        barcodePage->show();
        if (barcodePage->exec() == QDialog::Accepted) {
            QString codigoEscaneado = barcodePage->getLineEditText();
            if(existeCodBarras(codigoEscaneado)){
                QMessageBox::warning(this, "Aviso", "Já existe um produto com esse código cadastrado.");
            }else{
                addProdSemCodBarras(QString::number(id_produto_nota), codigoEscaneado);
            }


        }
    }

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
    query.prepare("SELECT quantidade, descricao, preco, codigo_barras, un_comercial, ncm, csosn, pis "
                  "FROM produtos_nota WHERE id = :id");
    query.bindValue(":id", idProd);
    if(query.exec()){
        while(query.next()){
            quant = query.value(0).toString();
            desc = query.value(1).toString();
            preco = query.value(2).toString();
            cod_barras = query.value(3).toString();
            un_comercial = query.value(4).toString();
            ncm = query.value(5).toString();
            csosn = query.value(6).toString();
            pis = query.value(7).toString();

        }
    }else{
        qDebug() << "query addProdSemCodBarras nao rodou";
    }
    quant = portugues.toString(quant.toFloat());
    preco = portugues.toString(preco.toFloat());

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
    db.close();
}

