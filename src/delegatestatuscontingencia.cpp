#include "delegatestatuscontingencia.h"

DelegateStatusContingencia::DelegateStatusContingencia(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void DelegateStatusContingencia::paint(QPainter *painter,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    painter->save();

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    QString cstat = index.data(Qt::DisplayRole).toString();

    QString texto;
    QColor cor;

    if (cstat == "CONTINGENCIA_FALHA") {
        texto = "Falha na Retransmissão";
        cor = QColor(200, 0, 0); // vermelho
    } else if (cstat == "CONTINGENCIA") {
        texto = "Aguardando Reenvio";
        cor = QColor(255, 140, 0); // laranja
    } else {
        texto = cstat;
        cor = opt.palette.text().color();
    }

    opt.text = texto;
    opt.font.setBold(true);
    opt.palette.setColor(QPalette::Text, cor);

    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    painter->restore();
}
