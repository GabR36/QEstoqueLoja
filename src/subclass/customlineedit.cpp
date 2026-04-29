#include "customlineedit.h"
CustomLineEdit::CustomLineEdit(QWidget *parent) : QLineEdit(parent) {
    // Construtor
}
void CustomLineEdit::mousePressEvent(QMouseEvent *event)
{
    QLineEdit::mousePressEvent(event);
    selectAll();
}

void CustomLineEdit::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);
    QMetaObject::invokeMethod(this, "selectAll", Qt::QueuedConnection);
}
