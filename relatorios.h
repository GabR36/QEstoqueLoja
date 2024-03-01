#ifndef RELATORIOS_H
#define RELATORIOS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QItemSelection>
#include "mainwindow.h"
namespace Ui {
class relatorios;
}

class relatorios : public QWidget
{
    Q_OBJECT

public:
    explicit relatorios(QWidget *parent = nullptr);
    QSqlDatabase db = QSqlDatabase::database();
    ~relatorios();


private slots:
    void on_Btn_PdfGen_clicked();

private:
    Ui::relatorios *ui;
};

#endif // RELATORIOS_H
