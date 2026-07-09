#ifndef DELEGATESTATUSCONTINGENCIA_H
#define DELEGATESTATUSCONTINGENCIA_H

#include <QStyledItemDelegate>
#include <QPainter>

class DelegateStatusContingencia : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DelegateStatusContingencia(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif // DELEGATESTATUSCONTINGENCIA_H
