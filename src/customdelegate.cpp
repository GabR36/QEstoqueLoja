#include "customdelegate.h"
#include <QPainter>

CustomDelegate::CustomDelegate(QObject *parent) : QStyledItemDelegate(parent) {}


void CustomDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // Chama o método base para manter a aparência padrão
    QStyledItemDelegate::paint(painter, option, index);

    // Verifica se o valor da célula é zero
    if (index.data().toInt() <= 0) {
        QRect rect = option.rect;
        painter->save();
        painter->setPen(QPen(Qt::red, 2));  // Define a cor e a espessura da borda
        painter->drawRect(rect.adjusted(0, 0, -1, -1));  // Desenha a borda ao redor da célula
        painter->restore();
    }
}
