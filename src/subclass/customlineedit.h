#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>
#include <QLineEdit>
#include <QMouseEvent>
#include <QFocusEvent>

class CustomLineEdit : public QLineEdit
{
public:
    explicit CustomLineEdit(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

};

#endif // CUSTOMLINEEDIT_H
