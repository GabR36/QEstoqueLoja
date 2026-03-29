#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDoubleValidator>
#include <QLocale>
#include "services/config_service.h"

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

    void on_Btn_schemaPath_clicked();

    void on_Btn_certificadoPath_clicked();

    void on_label_35_linkActivated(const QString &link);

private:
    Ui::Config *ui;
    QSqlDatabase db = QSqlDatabase::database();
    QLocale portugues;
    Config_service *configService;
signals:
    void alterouConfig();
};

#endif // CONFIGURACAO_H
