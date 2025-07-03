#include "delegateprecovalidate.h"
#include <QLineEdit>
#include <QLocale>
#include <QPalette>
#include <QPainter>

DelegatePrecoValidate::DelegatePrecoValidate(QObject *parent)
    : QStyledItemDelegate{parent}
{}
QWidget *DelegatePrecoValidate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QLineEdit *editor = new QLineEdit(parent);
    QDoubleValidator *validator = new QDoubleValidator(0, 9999999, 2, editor);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setLocale(QLocale(QLocale::Portuguese, QLocale::Brazil));
    editor->setValidator(validator);
    editor->setStyleSheet("background-color: lightblue;");
    return editor;
}

void DelegatePrecoValidate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (!lineEdit) return;

    bool ok;
    double preco = index.model()->data(index, Qt::EditRole).toDouble(&ok);
    if (ok) {
        lineEdit->setText(QLocale(QLocale::Portuguese, QLocale::Brazil).toString(preco, 'f', 2));
    } else {
        lineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
    }
}

void DelegatePrecoValidate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (!lineEdit) return;

    QString texto = lineEdit->text();
    bool ok;
    double preco = QLocale(QLocale::Portuguese, QLocale::Brazil).toDouble(texto, &ok);

    if (ok) {
        model->setData(index, QLocale(QLocale::Portuguese, QLocale::Brazil).toString(preco, 'f', 2), Qt::EditRole);
    } else {
        model->setData(index, texto, Qt::EditRole);
    }

}

void DelegatePrecoValidate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void DelegatePrecoValidate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionCopy = option;


        painter->fillRect(option.rect, QColor(230, 240, 255)); // Azul claro permanente


        // Altera a cor de fundo para um tom suave de azul
        optionCopy.backgroundBrush = QBrush(QColor(230, 240, 255)); // Azul suave


    // Chama o método de pintura padrão

        QStyledItemDelegate::paint(painter, optionCopy, index);
}
