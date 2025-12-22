#ifndef JANELAEMAILCONTADOR_H
#define JANELAEMAILCONTADOR_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class JanelaEmailContador;
}

class JanelaEmailContador : public QDialog
{
    Q_OBJECT

public:
    explicit JanelaEmailContador(QWidget *parent = nullptr);
    ~JanelaEmailContador();

private slots:
    void on_Dedit_Inicio_dateChanged(const QDate &date);

    void on_Dedit_Fim_dateChanged(const QDate &date);

private:
    Ui::JanelaEmailContador *ui;
    QSqlDatabase db;
    void atualizarContadores();
};

#endif // JANELAEMAILCONTADOR_H
