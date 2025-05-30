#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDoubleValidator>
#include <QLocale>

namespace Ui {
class Config;
}

class Configuracao : public QWidget
{
    Q_OBJECT

public:
    explicit Configuracao(QWidget *parent = nullptr);
    ~Configuracao();

private slots:

    void on_Btn_Aplicar_clicked();

    void on_Btn_Cancelar_clicked();

    void on_Btn_LogoEmpresa_clicked();

private:
    Ui::Config *ui;
    QSqlDatabase db = QSqlDatabase::database();
    QLocale portugues;
};

#endif // CONFIGURACAO_H
