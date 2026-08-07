#include "delegatehora.h"

DelegateHora::DelegateHora(QObject *parent) : QStyledItemDelegate(parent) {}

QString DelegateHora::displayText(const QVariant &value, const QLocale &locale) const {
    if (value.canConvert<QDateTime>()) {
        QDateTime dateTime = value.toDateTime();
        if (dateTime.isValid()) {
            // Formata a data e hora no formato "dd/MM/yyyy HH:mm:ss"
            return dateTime.toString("dd/MM/yyyy HH:mm:ss");
        }
    }
    // Caso contrário, chama o método base para a exibição padrão
    return QStyledItemDelegate::displayText(value, locale);
}
