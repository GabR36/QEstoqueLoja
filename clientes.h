#ifndef CLIENTES_H
#define CLIENTES_H

#include <QWidget>

namespace Ui {
class Clientes;
}

class Clientes : public QWidget
{
    Q_OBJECT

public:
    explicit Clientes(QWidget *parent = nullptr);
    ~Clientes();

private:
    Ui::Clientes *ui;
};

#endif // CLIENTES_H
