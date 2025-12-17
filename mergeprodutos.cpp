#include "mergeprodutos.h"
#include "ui_mergeprodutos.h"
#include <QVariantMap>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlDatabase>
#include "delegatesugerido.h"

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

    QStringList ordemPreferida = {
        "descricao",
        "quantidade",
        "preco_fornecedor",
        "porcent_lucro",
        "preco",
        "aliquota_imposto",
        "pis",
        "csosn",
        "ncm",
        "un_comercial"
    };

    std::sort(campos.begin(), campos.end(),
      [&](const QString &a, const QString &b) {

          int idxA = ordemPreferida.indexOf(a);
          int idxB = ordemPreferida.indexOf(b);

          // Ambos estão na lista de prioridade
          if (idxA != -1 && idxB != -1)
              return idxA < idxB;

          // Só A está na prioridade → A vem primeiro
          if (idxA != -1)
              return true;

          // Só B está na prioridade → B vem primeiro
          if (idxB != -1)
              return false;

          // Nenhum está → ordena alfabeticamente
          return a < b;
    });

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
        { "porcent_lucro", "Pctg Lucro" },
        { "preco_fornecedor", "Preço Fornecedor" }
    };

    for (int row = 0; row < campos.size(); ++row) {
        const QString &chave = campos[row];

        QString legivel = mapaHeadersLegiveis.value(chave, chave);
        modeloComparacaoProd->setHeaderData(row, Qt::Vertical, legivel);
        modeloComparacaoProd->setHeaderData(row, Qt::Vertical, chave, Qt::UserRole);

        QStandardItem *item = new QStandardItem();
        item->setData(produto1.value(chave), Qt::EditRole);
        modeloComparacaoProd->setItem(row, 0, item);

        QStandardItem *item2 = new QStandardItem();
        item2->setData(produto2.value(chave), Qt::EditRole);
        modeloComparacaoProd->setItem(row, 1, item2);

        QStandardItem *item3 = new QStandardItem();
        item3->setData(sugerido.value(chave), Qt::EditRole);
        modeloComparacaoProd->setItem(row, 2, item3);
    }

    ui->Tview_ComparacaoProd->setModel(modeloComparacaoProd);
    ui->Tview_ComparacaoProd->horizontalHeader()->setStretchLastSection(true);
    ui->Tview_ComparacaoProd->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->Tview_ComparacaoProd->setWordWrap(true);
    ui->Tview_ComparacaoProd->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    int linhaQuantidade = -1;

    for (int row = 0; row < modeloComparacaoProd->rowCount(); ++row) {
        QString chave = modeloComparacaoProd
                            ->headerData(row, Qt::Vertical, Qt::UserRole)
                            .toString();

        if (chave == "quantidade") {
            linhaQuantidade = row;
            break;
        }
    }
    ui->Tview_ComparacaoProd->setItemDelegate(
        new DelegateSugerido(ui->Tview_ComparacaoProd)
        );



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

