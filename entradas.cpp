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

Entradas::Entradas(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Entradas)
{
    ui->setupUi(this);
    empresaValues = Configuracao::get_All_Empresa_Values();
    db = QSqlDatabase::database();
    carregarTabela();
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

        manifestdfe->consultarEBaixarXML();
        QMessageBox::information(this, "Resposta", "Consultado");
        carregarTabela();
    }else{
        QMessageBox::information(this, "Erro", "Espere uma hora para consultar");
    }
}

void Entradas::carregarTabela()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);

    model->setQuery(R"(
        SELECT
            c.nome AS emitente,
            n.valor_total AS valor,
            n.dhemi AS emissao,
            n.cnpjemit AS cnpj,
            n.modelo AS modelo,
            n.chnfe AS chave,
            n.cstat AS cstat
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
}

