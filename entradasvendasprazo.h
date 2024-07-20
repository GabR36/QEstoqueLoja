#ifndef ENTRADASVENDASPRAZO_H
#define ENTRADASVENDASPRAZO_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QLocale>

namespace Ui {
class EntradasVendasPrazo;
}

class EntradasVendasPrazo : public QDialog
{
    Q_OBJECT

public:
    explicit EntradasVendasPrazo(QWidget *parent = nullptr, QString id_venda = "1");
    ~EntradasVendasPrazo();
    float valorDevidoGlobal;
    void atualizarTabelaPag();

private slots:
    void on_btn_AddValor_clicked();

    void onPgmntFechado();

    void on_btn_DeletarEntrada_clicked();

private:
    Ui::EntradasVendasPrazo *ui;
    QLocale portugues;
    QSqlDatabase db = QSqlDatabase::database();
    QString idVenda, valorVenda, dataHoraVenda, clienteVenda;
    float valor_Venda;
    QSqlQueryModel *modeloEntradas = new QSqlQueryModel;
    QString idEntradaSelec; //id global da entrada selecionada da tabela




};

#endif // ENTRADASVENDASPRAZO_H
