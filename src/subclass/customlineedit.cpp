#include "customlineedit.h"
CustomLineEdit::CustomLineEdit(QWidget *parent) : QLineEdit(parent) {
    // Construtor
}
void CustomLineEdit::mousePressEvent(QMouseEvent *event)
{
    QLineEdit::mousePressEvent(event); // Chama a implementação base do QLineEdit
    selectAll();
}
