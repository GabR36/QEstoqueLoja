#ifndef CONFIG_H
#define CONFIG_H

#include <QWidget>

namespace Ui {
class Config;
}

class Config : public QWidget
{
    Q_OBJECT

public:
    explicit Config(QWidget *parent = nullptr);
    ~Config();

private:
    Ui::Config *ui;
};

#endif // CONFIG_H
