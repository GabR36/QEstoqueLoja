#ifndef ENTRADAS_H
#define ENTRADAS_H

#include <QWidget>
#include <QSqlDatabase>
#include "nota/ACBrNFe.h"
#include <QLocale>

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
    void on_EntradaSelecionada(const QModelIndex &current, const QModelIndex &previous);


    void on_Tview_ProdutosNota_customContextMenuRequested(const QPoint &pos);

private:
    Ui::Entradas *ui;
    QSqlDatabase db;
    QMap<QString, QString> empresaValues;
    ACBrNFe *nfe;
    QMap<QString, QString> financeiroValues;
    QMap<QString, QString> produtoValues;
    QLocale portugues;


    void salvarRegistroDFe(const QString &nome_emitente, const QString &data_emissao, const QString &vnf, const QString &nsu, const QString &tipo, const QString &chave, const QString &cnpj, const QString &situacao, const QString &xml, const QString &data_recebimento);
    void carregarTabela();
    QString converterDataSefaz(const QString &data);
    void carregarProdutosDaNota(int id_nf);
    bool existeCodBarras(QString codigo);
    void addProdSemCodBarras(QString idProd, QString codBarras);
signals:
    void produtoAdicionado();
};

#endif // ENTRADAS_H
