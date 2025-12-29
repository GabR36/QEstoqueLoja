#ifndef DELEGATEPRECOF2_H
#define DELEGATEPRECOF2_H
#include <QStyledItemDelegate>
#include <QLocale>
#include <QDoubleSpinBox>

class DelegatePrecoF2 : public QStyledItemDelegate
{
public:
    DelegatePrecoF2(QObject *parent = nullptr);
    QString displayText(const QVariant &value, const QLocale &locale) const override;

private:
    QLocale portugues;
};

#endif // DELEGATEPRECOF2_H
