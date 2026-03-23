#ifndef JANELAEMAILCONTADOR_H
#define JANELAEMAILCONTADOR_H

#include <QDialog>
#include <QDateTime>
#include "services/email_service.h"

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
    void on_pushButton_clicked();

private:
    Ui::JanelaEmailContador *ui;
    Email_service emailServ;
    void atualizarContadores();
};

#endif // JANELAEMAILCONTADOR_H
