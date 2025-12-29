#ifndef DELEGATELOCKCOL_H
#define DELEGATELOCKCOL_H

#include <QStyledItemDelegate>
#include <QWidget>
#include <QModelIndex>
#include <QStyleOption>
#include <QPainter>
#include <QItemDelegate>

class DelegateLockCol : public QItemDelegate
{
public:
    DelegateLockCol(int nonEditableColumn, QObject *parent = nullptr);
protected:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
     int nonEditableColumn;
};

#endif // DELEGATELOCKCOL_H
