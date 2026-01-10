#include "delegateambiente.h".h"

DelegateAmbiente::DelegateAmbiente(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void DelegateAmbiente::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    painter->save();

    // mantém seleção funcionando
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    int valor = index.data(Qt::DisplayRole).toInt();

    QString texto;
    QColor cor;

    if (valor == 0) {
        texto = "Homologação";
        cor = QColor(255, 140, 0); // laranja
    } else if (valor == 1) {
        texto = "Produção";
        cor = QColor(0, 150, 0); // verde
    } else {
        texto = index.data(Qt::DisplayRole).toString();
        cor = opt.palette.text().color();
    }

    opt.text = texto;
    opt.palette.setColor(QPalette::Text, cor);

    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    painter->restore();
}
