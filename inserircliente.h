#ifndef INSERIRCLIENTE_H
#define INSERIRCLIENTE_H

#include <QWidget>
#include <QSqlDatabase>

namespace Ui {
class InserirCliente;
}

class InserirCliente : public QWidget
{
    Q_OBJECT

public:
    explicit InserirCliente(QWidget *parent = nullptr);
    ~InserirCliente();

private slots:

    void on_Btn_Cancelar_clicked();

    void on_Btn_Inserir_clicked();

private:
    Ui::InserirCliente *ui;
    QSqlDatabase db = QSqlDatabase::database();
};

#endif // INSERIRCLIENTE_H
