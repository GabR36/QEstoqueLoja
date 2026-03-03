#ifndef CLIENTES_H
#define CLIENTES_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QItemSelection>
#include <QLocale>
#include <QSqlQueryModel>
#include "services/cliente_service.h"
#include "services/vendas_service.h"
#include "services/financeiro_service.h"
#include "services/entradasvendas_service.h"

namespace Ui {
class Clientes;
}

class Clientes : public QWidget
{
    Q_OBJECT

public:
    explicit Clientes(QWidget *parent = nullptr);
    ~Clientes();

    void atualizarTableview();
    void atualizarInfosSinal();
    void atualizarTabelaClientes();
private slots:
    void on_Btn_Alterar_clicked();

    void on_Btn_Deletar_clicked();

    void on_Btn_Novo_clicked();

    void on_Btn_abrirCompras_clicked();

    void on_Ledit_Pesquisa_textChanged(const QString &arg1);

private:
    Ui::Clientes *ui;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQueryModel *model = nullptr;
    QLocale portugues;
    qlonglong IDCLIENTE = 0;
    void atualizarInfos(const QItemSelection &selected, const QItemSelection &);
    Cliente_service cliServ;
    Vendas_service vendaServ;
    Financeiro_service financeiroServ;
    EntradasVendas_service entradaServ;
};

#endif // CLIENTES_H
