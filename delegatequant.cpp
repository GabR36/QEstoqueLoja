#include "delegatequant.h"
#include <QPainter>
#include <QPalette>


DelegateQuant::DelegateQuant(QObject *parent) : QItemDelegate(parent){}

QWidget *DelegateQuant::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Cria um editor de texto (QLineEdit)
    QLineEdit *editor = new QLineEdit(parent);

    // Configura o validador para permitir apenas números positivos com vírgula
    QDoubleValidator *validator = new QDoubleValidator(0.0, 1000000.0, 2, editor); // Permite 2 casas decimais
    validator->setNotation(QDoubleValidator::StandardNotation);  // Notação padrão, por exemplo: 1234.56 ou 1234,56
    editor->setValidator(validator);
    editor->setStyleSheet("background-color: lightblue;");

    return editor;  // Retorna o editor para edição
}

void DelegateQuant::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString currentText = index.data().toString();
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(currentText);  // Preenche o editor com o valor da célula
}

void DelegateQuant::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    model->setData(index, lineEdit->text());  // Atualiza o modelo com o novo valor
}

void DelegateQuant::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);  // Define a posição e o tamanho do editor na célula
}

void DelegateQuant::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionCopy = option;


    painter->fillRect(option.rect, QColor(230, 240, 255)); // Azul claro permanente


    // Altera a cor de fundo para um tom suave de azul
    optionCopy.backgroundBrush = QBrush(QColor(230, 240, 255)); // Azul suave


    // Chama o método de pintura padrão

    QItemDelegate ::paint(painter, optionCopy, index);
}


