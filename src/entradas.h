#ifndef ENTRADAS_H
#define ENTRADAS_H

#include <QWidget>
#include <QSqlDatabase>
#include "nota/ACBrNFe.h"
#include <QLocale>
#include "nota/nfeacbr.h"
#include <QSqlQueryModel>

struct Cliente{
    QString nome;
    QString email;
    QString telefone;
    QString endereco;
    QString cpfcnpj;
    QString data_nasc;
    QString data_cadas;
    bool ehpf;
    QString numero_end;
    QString bairro;
    QString xmun;
    QString cmun;
    QString uf;
    QString cep;
    int indiedest;
    QString ie;
};

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

    void on_DateEdt_De_userDateChanged(const QDate &date);

    void on_DateEdt_Ate_userDateChanged(const QDate &date);

private:
    Ui::Entradas *ui;
    QSqlDatabase db;
    QMap<QString, QString> empresaValues;
    QMap<QString, QString> financeiroValues;
    QMap<QString, QString> produtoValues;
    QLocale portugues;
    NfeACBR *nfe;
    qlonglong id_nf_selec;
    QString lastInsertedIDNfDevol;
    QString cstatRetornado;
    QSqlQueryModel *modelEntradas;


    void salvarRegistroDFe(const QString &nome_emitente, const QString &data_emissao, const QString &vnf, const QString &nsu, const QString &tipo, const QString &chave, const QString &cnpj, const QString &situacao, const QString &xml, const QString &data_recebimento);
    void carregarTabela();
    QString converterDataSefaz(const QString &data);
    void carregarProdutosDaNota(qlonglong id_nf);
    bool existeCodBarras(QString codigo);
    void addProdSemCodBarras(QString idProd, QString codBarras);
    void devolverProdutos(QList<qlonglong> &idsProduto);
    QString salvarDevolucaoNf(QString retornoEnvio, QString idnf, NfeACBR *devolNfe,
                              QList<qlonglong> &idsProduto);
    void atualizarProdutoNotaAoDevolver(QString idNfDevol, QList<qlonglong> &idsProduto);
    double calcularPrecoItemSN(const QString &xmlPath, int nItem);
    void addProdComCodBarras(QString idProd, QString codBarras);
    void atualizarProdutoNotaAdicionado(QString idProd);
    void enviarEmailNFe(QString nomeCliente, QString emailCliente, QString xmlPath, std::string pdfDanfe, QString cnpj);
    void atualizarTabela(QString whereSql = "");

signals:
    void produtoAdicionado();
};

#endif // ENTRADAS_H
