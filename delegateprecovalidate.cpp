#include "delegateprecovalidate.h"

DelegatePrecoValidate::DelegatePrecoValidate(QObject *parent)
    : QStyledItemDelegate{parent}
{}
QWidget *DelegatePrecoValidate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    // Criando um editor numérico para o preço
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setDecimals(2);  // Definir 2 casas decimais
    editor->setMinimum(0.01); // Preço mínimo permitido
    editor->setMaximum(99999.99); // Preço máximo permitido
    editor->setSingleStep(1.00); // Incremento ao usar setas

    return editor;
}

void DelegatePrecoValidate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::EditRole).toDouble();
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
    if (spinBox)
        spinBox->setValue(value);
}

void DelegatePrecoValidate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
    if (spinBox) {
        double value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    }
}

void DelegatePrecoValidate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
