#pragma once

#include <QStyledItemDelegate>

class DelegateSugerido : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DelegateSugerido(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QString displayText(const QVariant &value, const QLocale &locale) const override;
};
