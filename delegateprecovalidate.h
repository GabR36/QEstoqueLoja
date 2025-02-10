#ifndef DELEGATEPRECOVALIDATE_H
#define DELEGATEPRECOVALIDATE_H

#include <QStyledItemDelegate>

#include <QValidator>

class DelegatePrecoValidate : public QStyledItemDelegate
{
public:
    explicit DelegatePrecoValidate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // DELEGATEPRECOVALIDATE_H
