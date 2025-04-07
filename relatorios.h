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

    QMap<QString, int> buscarVendasPorMes();

    QStringList buscarAnosDisponiveis();
    QMap<QString, int> buscarVendasPorMesAno(const QString &ano);
protected:
    QMap<QString, int> buscarVendasPorDiaMesAno(const QString &ano, const QString &mes);
    QMap<QString, int> buscarTopProdutosVendidos();
    QMap<QString, QPair<double, double> > buscarValorVendasPorMesAno(const QString &ano);
    QMap<QString, double> buscarValorVendasPorDiaMesAno(const QString &ano, const QString &mes);
private slots:
    void on_Btn_PdfGen_clicked();

    void on_Btn_CsvGen_clicked();

private:
    Ui::relatorios *ui;
    void conectarBancoDados();
    QLocale portugues;
    QStringList meses = {"01 - Janeiro", "02 - Fevereiro", "03 - Março", "04 - Abril", "05 - Maio",
                         "06 - Junho", "07 - Julho", "08 - Agosto", "09 - Setembro",
                         "10 - Outubro", "11 - Novembro", "12 - Dezembro"};
    void configurarJanelaQuantVendas();
    void configurarJanelaValorVendas();
    void configurarJanelaTopProdutosVendas();
};

#endif // RELATORIOS_H
