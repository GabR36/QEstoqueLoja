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

private slots:
    void on_btn_AddValor_clicked();

private:
    Ui::EntradasVendasPrazo *ui;
    QLocale portugues;
    QSqlDatabase db = QSqlDatabase::database();
    QString idVenda, valorVenda, dataHoraVenda, clienteVenda;
    void atualizarTabelaPag();
    float valor_Venda;
    QSqlQueryModel *modeloEntradas = new QSqlQueryModel;




};

#endif // ENTRADASVENDASPRAZO_H