#include "delegatesugerido.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPainter>
#include "mainwindow.h"
#include <QStyledItemDelegate>
#include "util/NfUtilidades.h"

DelegateSugerido::DelegateSugerido(QObject *parent)
    : QStyledItemDelegate(parent)
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
        QDoubleSpinBox *dsb = new QDoubleSpinBox(parent);
        dsb->setDecimals(2);
        dsb->setMinimum(0.0);
        dsb->setMaximum(1000000.0);
        dsb->setSingleStep(0.01);  // passo fino
        return dsb;
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

        for (int i = 0; i < unidadesComerciaisCount; ++i) {
            cb->addItem(unidadesComerciais[i]);
        }
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

    if (auto dsb = qobject_cast<QDoubleSpinBox *>(editor))
        dsb->setValue(valor.toDouble());

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


}

void DelegateSugerido::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.column() == 2) {
        painter->fillRect(option.rect, QColor(230, 240, 255));
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QString DelegateSugerido::displayText(const QVariant &value,
                                      const QLocale &locale) const
{
    if (value.metaType().id() == QMetaType::Double) {
        return locale.toString(value.toDouble(), 'f', 2);
    }
    return QStyledItemDelegate::displayText(value, locale);
}
