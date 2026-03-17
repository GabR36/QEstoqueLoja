#ifndef ENTRADAS_H
#define ENTRADAS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QLocale>
#include <QSqlQueryModel>
#include "services/dfe_service.h"
#include "services/config_service.h"
#include "services/notafiscal_service.h"
#include "services/produtonota_service.h"
#include "services/Produto_service.h"
#include "services/fiscalemitter_service.h"
#include "services/cliente_service.h"

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
    qlonglong id_nf_selec;
    QSqlQueryModel *modelEntradas;
    QSqlQueryModel *modelProdutosNota;
    ConfigDTO configDTO;
    Dfe_service dfeServ;
    NotaFiscal_service notaServ;
    ProdutoNota_service prodNotaServ;
    Produto_Service prodServ;
    FiscalEmitter_service fiscalEmitterServ;
    Cliente_service clienteServ;


    void carregarTabela();
    void carregarProdutosDaNota(qlonglong id_nf);
    void addProdSemCodBarras(QString idProd, QString codBarras);
    void devolverProdutos(QList<ProdutoNotaDTO> &produtosNota);
    double calcularPrecoItemSN(const QString &xmlPath, int nItem);
    void addProdComCodBarras(QString idProd, QString codBarras);
    void atualizarProdutoNotaAdicionado(QString idProd);
    void enviarEmailNFe(QString nomeCliente, QString emailCliente, QString xmlPath, std::string pdfDanfe, QString cnpj);
    void atualizarTabela(const QString &de = "", const QString &ate = "");

signals:
    void produtoAdicionado();
};

#endif // ENTRADAS_H
