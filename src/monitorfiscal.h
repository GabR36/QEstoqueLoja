#ifndef MONITORFISCAL_H
#define MONITORFISCAL_H

#include <QWidget>
#include "menuitem.h"
#include <QSqlDatabase>
#include <QSqlQueryModel>


namespace Ui {
class MonitorFiscal;
}

class MonitorFiscal : public QWidget
{
    Q_OBJECT

public:
    explicit MonitorFiscal(QWidget *parent = nullptr);
    ~MonitorFiscal();

private slots:
    void on_Btn_Danfe_clicked();

private:
    Ui::MonitorFiscal *ui;
    QVector<MenuItem*> m_items;
    void selectItem(MenuItem *item);
    QSqlDatabase db;
    QSqlQueryModel *modelSaida;

    void abrirSaida();
    void carregarTabelaSaida();
    void abrirDevolucao();
    void carregarTabelaDevolucao();
    void AtualizarTabelaNotas(QString whereSql);
    void abrirEntrada();
    void AtualizarTabelaEventos(QString whereSql);
    void abrirEventos();
};

#endif // MONITORFISCAL_H
