#ifndef DELEGATEQUANT_H
#define DELEGATEQUANT_H

#include <QItemDelegate>
#include <QDoubleValidator>
#include <QLineEdit>

class DelegateQuant : public QItemDelegate
{
public:
    DelegateQuant(QObject *parent = nullptr);
protected:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // DELEGATEQUANT_H
