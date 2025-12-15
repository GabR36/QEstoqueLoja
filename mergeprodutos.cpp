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
    QStandardItemModel *modeloComparacaoProd = new QStandardItemModel(this);
    this->modeloComparacaoProd = modeloComparacaoProd;

    QStringList campos = sugerido.keys();
    campos.sort();

    modeloComparacaoProd->setColumnCount(3);
    modeloComparacaoProd->setRowCount(campos.size());

    modeloComparacaoProd->setHeaderData(0, Qt::Horizontal, "Existente");
    modeloComparacaoProd->setHeaderData(1, Qt::Horizontal, "Novo");
    modeloComparacaoProd->setHeaderData(2, Qt::Horizontal, "Sugerido");

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

    for (int row = 0; row < campos.size(); ++row) {
        const QString &chave = campos[row];

        QString legivel = mapaHeadersLegiveis.value(chave, chave);
        modeloComparacaoProd->setHeaderData(row, Qt::Vertical, legivel);
        modeloComparacaoProd->setHeaderData(row, Qt::Vertical, chave, Qt::UserRole);

        modeloComparacaoProd->setItem(
            row, 0,
            new QStandardItem(produto1.value(chave).toString())
            );

        modeloComparacaoProd->setItem(
            row, 1,
            new QStandardItem(produto2.value(chave).toString())
            );

        modeloComparacaoProd->setItem(
            row, 2,
            new QStandardItem(sugerido.value(chave).toString())
            );
    }

    ui->Tview_ComparacaoProd->setModel(modeloComparacaoProd);
    ui->Tview_ComparacaoProd->horizontalHeader()->setStretchLastSection(true);
    ui->Tview_ComparacaoProd->resizeColumnsToContents();
    ui->Tview_ComparacaoProd->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

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
    const int colunaSugerido = 2; // Existente=0, Novo=1, Sugerido=2

    QVariantMap valoresSugeridos;

    for (int row = 0; row < modeloComparacaoProd->rowCount(); ++row) {

        QString chave = modeloComparacaoProd
                            ->headerData(row, Qt::Vertical, Qt::UserRole)
                            .toString();

        if (chave.isEmpty())
            continue;

        QStandardItem *item = modeloComparacaoProd->item(row, colunaSugerido);
        if (!item)
            continue;

        QVariant valor = item->data(Qt::DisplayRole);
        valoresSugeridos.insert(chave, valor);
    }

    qDebug() << "valoresSugeridos:" << valoresSugeridos;

    // Monta UPDATE
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

    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Banco nao abriu em btn_importar (mergeprodutos)";
            return;
        }
    }

    QSqlQuery query(db);
    query.prepare(sql);

    for (const QVariant &v : binds)
        query.addBindValue(v);

    query.addBindValue(idProduto);

    if (!query.exec()) {
        qDebug() << "Erro ao atualizar produto:" << query.lastError();
    }

    db.close();
    close();
}

