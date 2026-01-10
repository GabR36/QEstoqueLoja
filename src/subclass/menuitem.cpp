#include "menuitem.h"
#include <QHBoxLayout>
#include <QEasingCurve>

static constexpr int CLOSED_WIDTH = 60;
static constexpr int OPEN_WIDTH   = 150;
static constexpr int NORMAL_HEIGHT = 48;
static constexpr int ACTIVE_HEIGHT = 60;
static constexpr int MENU_WIDTH    = 140;

MenuItem::MenuItem(const QString& text, QWidget *parent)
    : QFrame(parent)
{
    setFixedWidth(MENU_WIDTH);
    setMinimumHeight(NORMAL_HEIGHT);
    setMaximumHeight(NORMAL_HEIGHT);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_button = new QPushButton(text, this);
    m_button->setCheckable(true);
    m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(m_button);
    layout->setContentsMargins(0, 0, 0, 0);

    m_anim = new QPropertyAnimation(this, "maximumHeight", this);
    m_anim->setDuration(200);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
}

void MenuItem::open()
{
    m_button->setChecked(true);

    m_anim->stop();
    m_anim->setStartValue(height());
    m_anim->setEndValue(ACTIVE_HEIGHT);
    m_anim->start();
}

void MenuItem::close()
{
    m_button->setChecked(false);

    m_anim->stop();
    m_anim->setStartValue(height());
    m_anim->setEndValue(NORMAL_HEIGHT);
    m_anim->start();
}
