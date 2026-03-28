#ifndef RELATORIOS_H
#define RELATORIOS_H

#include <QWidget>
#include <QSqlQueryModel>
#include <QItemSelection>
#include "mainwindow.h"
#include <QPainter>
#include <QFileDialog>
#include <QPdfWriter>
#include <QDesktopServices>
#include <QLocale>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QDate>
#include <QComboBox>
#include "services/config_service.h"
#include "services/relatorios_service.h"
#include "util/graficohelper.h"

namespace Ui {
class relatorios;
}

class relatorios : public QWidget
{
    Q_OBJECT

public:
    explicit relatorios(QWidget *parent = nullptr);
    ~relatorios();

protected:
    QMap<QString,QString> fiscalValues;

private slots:

private:
    Ui::relatorios *ui;
    QLocale portugues;
    ConfigDTO configDTO;
    Relatorios_service relatoriosServ;

    bool existeProdutoVendido();
    void configurarJanelaQuantVendas();
    void configurarJanelaValorVendas();
    void configurarJanelaTopProdutosVendas();
    void configurarJanelaFormasPagamentoAno();
    void configurarJanelaNFValor();
    void configurarJanelaProdutoLucroValor();
    void configurarJanelaInventario();

    static Agrupamento agrupFromCombo(QComboBox *cb, bool semDia = false);

    QChartView *chartViewAtual();
    void exportarPdfAtual();
    void exportarCsvAtual();
};

#endif // RELATORIOS_H
