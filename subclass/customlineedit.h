#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>
#include <QLineEdit>
#include <QMouseEvent>

class CustomLineEdit : public QLineEdit
{
public:
    explicit CustomLineEdit(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *event) override;

};

#endif // CUSTOMLINEEDIT_H
