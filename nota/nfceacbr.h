#ifndef NFCEACBR_H
#define NFCEACBR_H

#include <QObject>
#include "acbrmanager.h"
#include <QString>
#include <QMap>
#include <QSqlDatabase>
#include <QVector>
#include <QLocale>
#include <sstream>

class NfceACBR : public QObject
{
    Q_OBJECT
public:
    explicit NfceACBR(QObject *parent = nullptr);
    QString getVersaoLib();
    int getProximoNNF();
    void setNNF(int nNF);
    void setCliente(QString cpf, bool ehPf);
    void setProdutosVendidos(QList<QList<QVariant> > produtosVendidos, bool emitirTodos);
    void setPagamentoValores(QString formaPag, float desconto, float recebido, float troco,
                             float taxa);
    int getNNF();
    int getSerie();
    QString gerarEnviar();
    QString getXmlPath();
    double getVNF();
    QString getChaveNf();
private:
    ACBrNFe *nfce;
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

#endif // NFCEACBR_H
