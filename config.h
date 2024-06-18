#ifndef CONFIG_H
#define CONFIG_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDoubleValidator>

namespace Ui {
class Config;
}

class Config : public QWidget
{
    Q_OBJECT

public:
    explicit Config(QWidget *parent = nullptr);
    ~Config();

private slots:

    void on_Btn_Aplicar_clicked();

    void on_Btn_Cancelar_clicked();

private:
    Ui::Config *ui;
    QSqlDatabase db = QSqlDatabase::database();
};

#endif // CONFIG_H
