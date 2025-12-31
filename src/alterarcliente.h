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

private slots:
    void on_Btn_Cancelar_clicked();

    void on_Btn_Ok_clicked();

    void on_Btn_BuscaDados_clicked();

private:
    Ui::AlterarCliente *ui;
    QSqlDatabase db =  QSqlDatabase::database();
    QString id;
    bool atualizando;

signals:
    void clienteAtualizado();
};

#endif // ALTERARCLIENTE_H
