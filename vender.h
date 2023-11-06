#ifndef VENDER_H
#define VENDER_H

#include <QWidget>

namespace Ui {
class Vender;
}

class Vender : public QWidget
{
    Q_OBJECT

public:
    explicit Vender(QWidget *parent = nullptr);
    ~Vender();

private:
    Ui::Vender *ui;
};

#endif // VENDER_H
