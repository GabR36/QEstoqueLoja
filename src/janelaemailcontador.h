#ifndef JANELAEMAILCONTADOR_H
#define JANELAEMAILCONTADOR_H

#include <QDialog>
#include <QSqlDatabase>
#include "services/config_service.h"
#include <QDateTime>

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
    QSqlDatabase db;
    void atualizarContadores();
    QMap<QString, QStringList> xmlsPorPasta;
    void enviarEmailContador(QString zip, QDate dtIni, QDate dtFim);
    ConfigDTO configDTO;
    void enviarEmailContador(QString zip, QDate dtIni, QDate dtFim, QString pdfPath);
    void gerarResumoPdf(const QString &filePath, QDateTime dtIni, QDateTime dtFim);
    Config_service confServ;
};

#endif // JANELAEMAILCONTADOR_H
