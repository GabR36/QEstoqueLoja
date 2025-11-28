#ifndef ENTRADAS_H
#define ENTRADAS_H

#include <QWidget>
#include <QSqlDatabase>
#include "nota/ACBrNFe.h"

namespace Ui {
class Entradas;
}

class Entradas : public QWidget
{
    Q_OBJECT

public:
    explicit Entradas(QWidget *parent = nullptr);
    ~Entradas();

private slots:
    void on_Btn_ConsultarDF_clicked();

private:
    Ui::Entradas *ui;
    QSqlDatabase db;
    QMap<QString, QString> empresaValues;
    ACBrNFe *nfe;



    void salvarRegistroDFe(const QString &nome_emitente, const QString &data_emissao, const QString &vnf, const QString &nsu, const QString &tipo, const QString &chave, const QString &cnpj, const QString &situacao, const QString &xml, const QString &data_recebimento);
    void carregarTabela();
};

#endif // ENTRADAS_H
