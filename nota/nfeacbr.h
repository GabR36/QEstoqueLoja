#ifndef NFEACBR_H
#define NFEACBR_H

#include <QObject>
#include "acbrmanager.h"
#include <QString>
#include <QMap>
#include <QSqlDatabase>
#include <QVector>
#include <QLocale>
#include <sstream>

class NfeACBR : public QObject
{
    Q_OBJECT
public:
    explicit NfeACBR(QObject *parent = nullptr);
    QString getVersaoLib();
    int getProximoNNF();
    void setNNF(int nNF);
    void setProdutosVendidos(QList<QList<QVariant> > produtosVendidos, bool emitirTodos);
    void setPagamentoValores(QString formaPag, float desconto, float recebido, float troco,
                             float taxa);
    int getNNF();
    int getSerie();
    QString gerarEnviar();
    QString getXmlPath();
    double getVNF();
    void setCliente(bool ehPf, QString cpf, QString nome, int indiedest, QString email,
                    QString lgr, QString nro, QString bairro, QString cmun, QString xmun,
                    QString uf, QString cep, QString ie);
    QString getChaveNf();
private:
    ACBrNFe *nfe;
    QSqlDatabase db;
    QLocale portugues;
    std::stringstream ini;
    QMap<QString, QString> fiscalValues;
    QMap<QString, QString> empresaValues;

    QString caminhoXml;
    std::string tpAmb, cuf, cnf, cnpjEmit;
    std::string nnf = "1";
    int mod, tpEmis;
    QString serieNf;

    QString cpfCliente;
    bool ehPfCliente, emitirApenasNf;
    int quantProds;
    QVector<float> vTotTribProduto;
    QVector<float> descontoProd;
    QList<QList<QVariant>>listaProdutos;
    double descontoNf,trocoNf,vPagNf, vNf;
    QString indPagNf,tPagNf;
    float taxaPercentual;
    double totalGeral;
    QString nomeCli, cpfCli, emailCli, lgrCli, nroCli, bairroCli,
        cmunCli, xmunCli, ufCli, cepCli, ieCli;
    bool ehPfCli = false;
    int indiedestCli = 0;
    bool usarIBS;

    void carregarConfig();
    void ide();
    void emite();
    void dest();
    void carregarProds();
    void total();
    void transp();
    void pag();
    void infRespTec();
    void ibscbsTotais();


    bool isValidGTIN(const QString &gtin);
    void aplicarDescontoTotal(float descontoTotal);
    float corrigirTaxa(float taxaAntiga, float desconto);
    void aplicarAcrescimoProporcional(float taxaPercentual);

signals:
};

#endif // NFEACBR_H
