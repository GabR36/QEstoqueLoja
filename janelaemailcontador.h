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

    void on_pushButton_clicked();

private:
    Ui::JanelaEmailContador *ui;
    QSqlDatabase db;
    void atualizarContadores();
    QMap<QString, QStringList> xmlsPorPasta;
    QMap<QString, QString> contadorValues;
    QMap<QString, QString> empresaValues;
    QMap<QString, QString> fiscalValues;
    void enviarEmailContador(QString zip, QDateTime dtIni, QDateTime dtFim);
};

#endif // JANELAEMAILCONTADOR_H
