#pragma once

#include <QStyledItemDelegate>
#include <QPainter>

class DelegateAmbiente : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DelegateAmbiente(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};
