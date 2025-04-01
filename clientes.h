#ifndef CLIENTES_H
#define CLIENTES_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QItemSelection>
#include <QLocale>

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
    int getQuantCompras(int idCliente);
    QString getDataUltimoPagamento(int idCliente);
    double getValorUltimoPagamento(int idCliente);
    double getValorDevido(int idCliente);
private slots:
    void on_Btn_Alterar_clicked();

    void on_Btn_Deletar_clicked();

    void on_Btn_Novo_clicked();

    void on_Btn_abrirCompras_clicked();

private:
    Ui::Clientes *ui;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQueryModel *model = new QSqlQueryModel;
    QLocale portugues;
    int IDCLIENTE = 0;
    void atualizarInfos(const QItemSelection &selected, const QItemSelection &);

};

#endif // CLIENTES_H
