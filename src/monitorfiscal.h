#ifndef MONITORFISCAL_H
#define MONITORFISCAL_H

#include <QWidget>
#include "menuitem.h"
#include <QSqlQueryModel>
#include "delegateambiente.h"
#include "delegatehora.h"
#include "services/notafiscal_service.h"
#include "services/eventofiscal_service.h"

enum class TipoVisualizacao {
    NotaFiscal,
    Evento
};


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
    QSqlQueryModel *modelSaida;
    QSqlQueryModel *modelEventos;
    TipoVisualizacao m_tipoAtual;
    DelegateAmbiente *delegateAmb;
    DelegateHora *delegateHora;
    NotaFiscal_service notaServ;
    EventoFiscal_service eventoServ;

    void abrirSaida();
    void abrirDevolucao();
    void carregarTabelaDevolucao();
    void AtualizarTabelaNotas(const QStringList &finalidades);
    void abrirEntrada();
    void AtualizarTabelaEventos();
    void abrirEventos();
};

#endif // MONITORFISCAL_H
