#ifndef DELEGATEPAGO_H
#define DELEGATEPAGO_H

#include <QStyledItemDelegate>

class DelegatePago : public QStyledItemDelegate
{
public:
    explicit DelegatePago(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

};

#endif // DELEGATEPAGO_H
