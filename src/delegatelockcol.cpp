#include "delegatelockcol.h"

DelegateLockCol::DelegateLockCol(int nonEditableColumn, QObject *parent)
    : QItemDelegate(parent), nonEditableColumn(nonEditableColumn)
{}

QWidget *DelegateLockCol::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == nonEditableColumn) {
        return nullptr;  // Impede a criação do editor
    }
    return QItemDelegate::createEditor(parent, option, index);
}

void DelegateLockCol::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == nonEditableColumn) {
        // Desenha a célula normalmente, mas sem permitir interação
        QStyleOptionViewItem opt = option;
        opt.state &= ~QStyle::State_MouseOver;  // Remove o estilo de mouse sobre a célula
        QItemDelegate::paint(painter, opt, index);
    } else {
        // Permite o comportamento padrão de pintura para células editáveis
        QItemDelegate::paint(painter, option, index);
    }
}

