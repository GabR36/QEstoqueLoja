#ifndef RELATORIOS_H
#define RELATORIOS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QItemSelection>
#include "mainwindow.h"
#include <QPainter>
#include <QFileDialog>
#include <QPdfWriter>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QtSql>
#include <QDesktopServices>
#include <QLocale>

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

    void on_Btn_CsvGen_clicked();

private:
    Ui::relatorios *ui;
    QLocale portugues;
};

#endif // RELATORIOS_H
