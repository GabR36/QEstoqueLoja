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
    QStringList meses = {"01 - Janeiro", "02 - Fevereiro", "03 - Março", "04 - Abril", "05 - Maio",
                         "06 - Junho", "07 - Julho", "08 - Agosto", "09 - Setembro",
                         "10 - Outubro", "11 - Novembro", "12 - Dezembro"};
    ConfigDTO configDTO;
    Relatorios_service relatoriosServ;

    bool existeProdutoVendido();
    void configurarJanelaQuantVendas();
    void configurarJanelaValorVendas();
    void configurarJanelaTopProdutosVendas();
    void configurarJanelaFormasPagamentoAno();
    void configurarJanelaNFValor();
    void configurarJanelaProdutoLucroValor();
};

#endif // RELATORIOS_H
