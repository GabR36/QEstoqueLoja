#ifndef ALTERARCLIENTE_H
#define ALTERARCLIENTE_H

#include <QWidget>
#include <QSqlDatabase>

namespace Ui {
class AlterarCliente;
}

class AlterarCliente : public QWidget
{
    Q_OBJECT

public:
    explicit AlterarCliente(QWidget *parent = nullptr, QString id = "1");
    ~AlterarCliente();

private:
    Ui::AlterarCliente *ui;
    QSqlDatabase db =  QSqlDatabase::database();
};

#endif // ALTERARCLIENTE_H
