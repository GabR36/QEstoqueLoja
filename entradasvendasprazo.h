#ifndef ENTRADASVENDASPRAZO_H
#define ENTRADASVENDASPRAZO_H

#include <QDialog>

namespace Ui {
class EntradasVendasPrazo;
}

class EntradasVendasPrazo : public QDialog
{
    Q_OBJECT

public:
    explicit EntradasVendasPrazo(QWidget *parent = nullptr);
    ~EntradasVendasPrazo();

private:
    Ui::EntradasVendasPrazo *ui;
};

#endif // ENTRADASVENDASPRAZO_H
