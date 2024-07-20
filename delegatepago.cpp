#include "delegatepago.h"
#include <QPainter>

DelegatePago::DelegatePago(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void DelegatePago::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    int value = index.data().toInt();
    QString text = (value == 1) ? "SIM" : "NÃO";
    QColor color = (value == 1) ? Qt::green : Qt::red;

    painter->save();
    QFont fonte;
    fonte.setBold(true);
    painter->setFont(fonte);
    painter->setPen(color);
    painter->drawText(option.rect, Qt::AlignCenter, text);

    painter->restore();
}
