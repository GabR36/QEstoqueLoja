#include "CustomDelegate.h"
#include <QPainter>

CustomDelegate::CustomDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void CustomDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.column() == 1) { // Assuming column 1 contains the quantity
        QVariant value = index.data(Qt::DisplayRole);
        if (value.toInt() <= 0) {
            painter->save();
            painter->setPen(Qt::red);
            painter->drawRect(option.rect.adjusted(0, 0, -1, -1)); // Adjusted to draw the border inside the cell
            painter->restore();
        }
    }
    QStyledItemDelegate::paint(painter, option, index);
}
