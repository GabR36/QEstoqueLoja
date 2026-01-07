#pragma once

#include <QFrame>
#include <QPushButton>
#include <QPropertyAnimation>

class MenuItem : public QFrame
{
    Q_OBJECT

public:
    explicit MenuItem(const QString& text, QWidget *parent = nullptr);

    void open();
    void close();

    QPushButton* button() const { return m_button; }

private:
    QPushButton* m_button;
    QPropertyAnimation* m_anim;
};
