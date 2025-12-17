#include "delegatesugerido.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPainter>
#include "mainwindow.h"

DelegateSugerido::DelegateSugerido(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *DelegateSugerido::createEditor(
    QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &index) const
{
    // SOMENTE coluna "Sugerido"
    if (index.column() != 2)
        return nullptr;

    QString chave = index.model()
                        ->headerData(index.row(), Qt::Vertical, Qt::UserRole)
                        .toString();

    // -------- quantidade --------
    if (chave == "quantidade") {
        QSpinBox *sb = new QSpinBox(parent);
        sb->setMinimum(0);
        sb->setMaximum(1000000);
        return sb;
    }

    // -------- preço / impostos --------
    if (chave == "preco" ||
        chave == "preco_fornecedor" ||
        chave == "porcent_lucro" ||
        chave == "aliquota_imposto" ||
        chave == "pis")  {

        QDoubleSpinBox *dsb = new QDoubleSpinBox(parent);
        dsb->setDecimals(2);
        dsb->setMinimum(0.0);
        dsb->setMaximum(1000000.0);
        dsb->setSingleStep(0.01);
        return dsb;
    }

    // -------- descrição --------
    if (chave == "descricao") {
        QLineEdit *le = new QLineEdit(parent);
        le->setMaxLength(255);
        return le;
    }

    // -------- unidade comercial --------
    if (chave == "un_comercial") {
        QComboBox *cb = new QComboBox(parent);
        cb->addItems({ "UN", "KG", "CX", "LT" });
        return cb;
    }

    // -------- CSOSN --------
    if (chave == "csosn") {
        QComboBox *cb = new QComboBox(parent);
        cb->addItems({ "101", "102", "103", "300", "400", "500" });
        return cb;
    }

    // -------- NCM --------
    if (chave == "ncm") {
        QLineEdit *le = new QLineEdit(parent);
        le->setInputMask("99999999");
        return le;
    }

    return nullptr;
}

void DelegateSugerido::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    QVariant valor = index.data(Qt::EditRole);

    if (auto sb = qobject_cast<QSpinBox *>(editor))
        sb->setValue(valor.toInt());

    else if (auto dsb = qobject_cast<QDoubleSpinBox *>(editor))
        dsb->setValue(valor.toDouble());

    else if (auto le = qobject_cast<QLineEdit *>(editor))
        le->setText(valor.toString());

    else if (auto cb = qobject_cast<QComboBox *>(editor)) {
        int idx = cb->findText(valor.toString());
        if (idx >= 0)
            cb->setCurrentIndex(idx);
    }
}

void DelegateSugerido::setModelData(QWidget *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex &index) const
{

    QString chave = index.model()
    ->headerData(index.row(), Qt::Vertical, Qt::UserRole)
        .toString();

    // ---------- QLineEdit ----------
    if (auto le = qobject_cast<QLineEdit *>(editor)) {

        QString texto = le->text();

        // 🔥 NORMALIZA AQUI
        if (chave == "descricao") {
            texto = MainWindow::normalizeText(texto);
        }

        model->setData(index, texto);
        return;
    }

    // ---------- QSpinBox ----------
    if (auto sb = qobject_cast<QSpinBox *>(editor)) {
        model->setData(index, sb->value());
        return;
    }

    // ---------- QDoubleSpinBox ----------
    if (auto dsb = qobject_cast<QDoubleSpinBox *>(editor)) {
        model->setData(index, dsb->value());
        return;
    }

    // ---------- QComboBox ----------
    if (auto cb = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, cb->currentText());
        return;
    }

    // if (auto sb = qobject_cast<QSpinBox *>(editor))
    //     model->setData(index, sb->value());

    // else if (auto dsb = qobject_cast<QDoubleSpinBox *>(editor))
    //     model->setData(index, dsb->value());

    // else if (auto le = qobject_cast<QLineEdit *>(editor))
    //     model->setData(index, le->text());

    // else if (auto cb = qobject_cast<QComboBox *>(editor))
    //     model->setData(index, cb->currentText());
}

void DelegateSugerido::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.column() == 2) {
        painter->fillRect(option.rect, QColor(230, 240, 255));
    }

    QItemDelegate::paint(painter, option, index);
}
