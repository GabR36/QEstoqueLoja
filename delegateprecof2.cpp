#include "delegateprecof2.h"

DelegatePrecoF2::DelegatePrecoF2(QObject *parent) : QStyledItemDelegate(parent){}

QString DelegatePrecoF2::displayText(const QVariant &value, const QLocale &locale) const{
    if (value.canConvert<float>()) {
        float preco = value.toFloat();

        return portugues.toString(preco,'f',2);

    }
    return QStyledItemDelegate::displayText(value, locale);

}


