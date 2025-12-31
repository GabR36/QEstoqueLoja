#include "delegatehora.h"

DelegateHora::DelegateHora(QObject *parent) : QStyledItemDelegate(parent) {}

QString DelegateHora::displayText(const QVariant &value, const QLocale &locale) const {
    if (value.canConvert<QDate>()) {
        QDate date = value.toDate();
        // Verifica se a data está no formato errado e corrige se necessário
        if (date.isValid()) {
            // Formata a data no formato "dd/MM/yyyy"
            return date.toString("dd/MM/yyyy");
        }
    }
    // Caso contrário, chama o método base para a exibição padrão
    return QStyledItemDelegate::displayText(value, locale);
}
