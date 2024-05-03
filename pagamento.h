#ifndef PAGAMENTO_H
#define PAGAMENTO_H

#include <QDialog>

namespace Ui {
class pagamento;
}

class pagamento : public QDialog
{
    Q_OBJECT

public:
    explicit pagamento(QWidget *parent = nullptr);
    ~pagamento();

private:
    Ui::pagamento *ui;
};

#endif // PAGAMENTO_H
