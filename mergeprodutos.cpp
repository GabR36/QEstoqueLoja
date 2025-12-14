#include "mergeprodutos.h"
#include "ui_mergeprodutos.h"
#include <QVariantMap>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlDatabase>

MergeProdutos::MergeProdutos(QVariantMap produto1, QVariantMap produto2,
                             QVariantMap sugerido, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MergeProdutos)
{
    ui->setupUi(this);
    idProduto = produto1["id"].toString();
    nfProduto = produto1["nf"].toBool();
    db = QSqlDatabase::database();
    QStandardItemModel *modeloComparacaoProd = new QStandardItemModel;
    this->modeloComparacaoProd = modeloComparacaoProd;
    QStringList headersOriginais = sugerido.keys();
    headersOriginais.sort();
    modeloComparacaoProd->setColumnCount(headersOriginais.size());

    const QMap<QString, QString> mapaHeadersLegiveis = {
        { "aliquota_imposto", "Alíquota de imposto" },
        { "quantidade", "Quantidade" },
        { "descricao", "Descrição" },
        { "preco", "Preço" },
        { "un_comercial", "Unidade Comercial" },
        { "ncm", "NCM" },
        { "csosn", "CSOSN" },
        { "pis", "PIS" },
    };

    for (int col = 0; col < headersOriginais.size(); ++col) {
        const QString &chave = headersOriginais[col];

        QString legivel = mapaHeadersLegiveis.value(chave, chave);

        modeloComparacaoProd->setHeaderData(
            col,
            Qt::Horizontal,
            legivel,
            Qt::DisplayRole
            );

        modeloComparacaoProd->setHeaderData(
            col,
            Qt::Horizontal,
            chave,
            Qt::UserRole
            );
    }

    QList<QStandardItem*> linhaSugerido;
    QList<QStandardItem*> linhaProd1;
    QList<QStandardItem*> linhaProd2;

    for (const QString &chave : headersOriginais) {
        QVariant valor = sugerido.value(chave);
        linhaSugerido << new QStandardItem(valor.toString());
        valor = produto1.value(chave);
        linhaProd1 << new QStandardItem(valor.toString());
        valor = produto2.value(chave);
        linhaProd2 << new QStandardItem(valor.toString());
    }

    modeloComparacaoProd->appendRow(linhaProd1);
    modeloComparacaoProd->appendRow(linhaProd2);
    modeloComparacaoProd->appendRow(linhaSugerido);

    modeloComparacaoProd->setVerticalHeaderLabels({
        "Existente",
        "Novo",
        "Sugerido"
    });

    ui->Tview_ComparacaoProd->setModel(modeloComparacaoProd);
    ui->Tview_ComparacaoProd->horizontalHeader()->setStretchLastSection(true);
    ui->Tview_ComparacaoProd->resizeColumnsToContents();
    // int altura = ui->Tview_ComparacaoProd->horizontalHeader()->height();
    // for (int row = 0; row < modeloComparacaoProd->rowCount(); ++row) {
    //     altura += ui->Tview_ComparacaoProd->rowHeight(row);
    // }
    // if (ui->Tview_ComparacaoProd->horizontalScrollBar()->isVisible()) {
    //     altura += ui->Tview_ComparacaoProd->horizontalScrollBar()->height();
    // }
    // ui->Tview_ComparacaoProd->setFixedHeight(altura);
    // ui->Tview_ComparacaoProd->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

MergeProdutos::~MergeProdutos()
{
    delete ui;
}

void MergeProdutos::on_Btn_Cancelar_clicked()
{
    this->close();
}


void MergeProdutos::on_Btn_Importar_clicked()
{
    int linhaSugerido = 2;

    QVariantMap valoresSugeridos;

    for (int col = 0; col < modeloComparacaoProd->columnCount(); ++col) {
        QString chave = modeloComparacaoProd
                            ->headerData(col, Qt::Horizontal, Qt::UserRole)
                            .toString();

        QVariant valor = modeloComparacaoProd
                             ->item(linhaSugerido, col)
                             ->data(Qt::DisplayRole);

        valoresSugeridos.insert(chave, valor);
    }

    qDebug() << "valoresSugeridos: " << valoresSugeridos;

    // atualizar o produto no banco de dados

    QStringList sets;
    QVariantList binds;

    for (auto it = valoresSugeridos.constBegin(); it != valoresSugeridos.constEnd(); ++it) {
        sets << QString("%1 = ?").arg(it.key());
        binds << it.value();
    }

    if (!nfProduto) {
        sets << "nf = 1";
    }

    QString sql =
        "UPDATE produtos SET " + sets.join(", ") +
        " WHERE id = ?";

    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Banco nao abriu em btn_importar (mergeprodutos)";
        }
    }

    QSqlQuery query;
    query.prepare(sql);

    for (const QVariant &v : binds)
        query.addBindValue(v);

    query.addBindValue(idProduto);

    query.exec();

    db.close();

    this->close();
}

