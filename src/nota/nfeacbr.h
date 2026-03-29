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
#include "../services/config_service.h"
#include "../dto/NotaFiscal_dto.h"
#include "../services/notafiscal_service.h"
#include "../dto/ProdutoVendido_dto.h"
#include "../services/Produto_service.h"
#include "../dto/ProdutoParaNFe_dto.h"
#include "../dto/NFRetorno_dto.h"
#include "../dto/Cliente_dto.h"

class NfeACBR : public QObject
{
    Q_OBJECT
public:
    explicit NfeACBR(QObject *parent = nullptr, bool saida = 1, bool devolucao = 0);
    QString getVersaoLib();
    qlonglong getProximoNNF();
    void setNNF(int nNF);
    void setProdutosVendidos(QList<QList<QVariant> > produtosVendidos, bool emitirTodos);
    void setPagamentoValores(QString formaPag, float desconto, float recebido, float troco,
                             float taxa);
    qlonglong getNNF();
    int getSerie();
    QString gerarEnviar();
    QString getXmlPath();
    double getVNF();
    void setCliente(ClienteDTO cliente);
    QString getChaveNf();
    void setNfRef(QString chnfe);
    QString getTpAmb();
    QString getCnpjEmit();
    QString getCuf();
    QString getDhEmiConvertida();
    void setProdutosNota(QList<qlonglong> &idsProduto);

    std::string getPdfDanfe();
    void setProdutosVendidosNew(QList<ProdutoVendidoDTO> listaProds, bool emitirTodos);
    NFRetornoDTO gerarEnviarRetorno();
    void setRetornoForcado(const QString &retorno);
private:
    ACBrNFe *nfe;
    QSqlDatabase db;
    QLocale portugues;
    std::stringstream ini;
    QMap<QString, QString> fiscalValues;
    QMap<QString, QString> empresaValues;
    QMap<QString, QString> produtosValues;

    QString caminhoXml;
    std::string tpAmb, cuf, cnf, cnpjEmit;
    std::string nnf = "1";
    int mod, tpEmis;
    QString serieNf;
    ConfigDTO configDTO;

    QString cpfCliente;
    bool ehPfCliente, emitirApenasNf;
    int quantProds;
    QVector<float> vTotTribProduto;
    // QVector<float> descontoProd;
    double descontoNf,trocoNf,vPagNf, vNf = 0;
    QString indPagNf,tPagNf;
    float taxaPercentual;
    double totalGeral;
    QString nomeCli, cpfCli, emailCli, lgrCli, nroCli, bairroCli,
        cmunCli, xmunCli, ufCli, cepCli, ieCli;
    bool ehPfCli = false;
    int indiedestCli = 0;
    bool usarIBS;
    QString idDest;
    QString retornoForcado = "";

    QString natOp,tpNf,finNfe,cfop, refNfe;
    std::string dataHora;

    enum tipoNota{
        saidaNormal,
        devolucaoVenda,
        devolucaoFornecedor,
    };

    tipoNota tipo;
    QList<ProdutoParaNFeDTO> listaProdutosNFe;

    void carregarConfig();
    void ide();
    void nfRef();
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
    double corrigirTaxa(double taxaAntiga, double desconto);
    void aplicarAcrescimoProporcional(float taxaPercentual);

    QString cfopDevolucao(const QString &cfopOriginal);
    NotaFiscal_service notaServ;
    Produto_Service prodServ;

    double getSomaValorTotalProdutos();
signals:
};

#endif // NFEACBR_H
