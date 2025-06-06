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

    static QMap<QString, QString> get_All_Fiscal_Values();
    static QMap<QString, QString> get_All_Empresa_Values();
private slots:

    void on_Btn_Aplicar_clicked();

    void on_Btn_Cancelar_clicked();

    void on_Btn_LogoEmpresa_clicked();

    void on_Btn_schemaPath_clicked();

    void on_Btn_certificadoPath_clicked();

    void on_Btn_CertficadoAcPath_clicked();

private:
    Ui::Config *ui;
    QSqlDatabase db = QSqlDatabase::database();
    QLocale portugues;
};

#endif // CONFIGURACAO_H
