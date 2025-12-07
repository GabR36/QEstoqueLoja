#ifndef MANIFESTADORDFE_H
#define MANIFESTADORDFE_H

#include <QObject>
#include <sstream>
#include <QMap>
#include <QSqlDatabase>
#include "../nota/eventocienciaop.h"

struct ResumoNFe {
    QString chave;
    QString nome;
    QString cnpjEmit;
    QString schema;
    QString vnf;
    QString cstat;
    QString xml_path;
    QString nProt;
    QString dhEmi;
};

struct ProcNfe {
    QString chave;
    QString nsu;
    QString cnpjEmit;
    QString schema;
    QString vnf;
    QString cstat;
    QString xml_path;
    QString nProt;
    QString dhEmi;
    QString nome;
    QString cSitNfe;
};


struct Emitente {
    QString cnpj;
    QString nome;
    QString xLgr;
    QString nro;
    QString xBairro;
    QString xMun;
    QString cMun;
    QString uf;
    QString cep;
    QString ie;
};

struct NotaFiscal {
    QString cstat;
    qlonglong nnf = 0;
    QString serie;
    QString modelo;
    bool tp_amb = false;
    QString xml_path;
    double valor_total = 0.0;
    QString cnpjemit;
    QString chnfe;
    QString nprot;
    QString cuf;
};

struct ProdutoNota {
    float quant;
    QString desc;
    double preco;
    QString cod_barras;
    QString un_comercial;
    QString ncm;
    QString csosn;
    QString pis;
    QString cfop;
    float aliquota_imposto;
    QString nitem;
    qlonglong id_nf;
};

class ManifestadorDFe : public QObject
{
    Q_OBJECT
public:
    explicit ManifestadorDFe(QObject *parent = nullptr);
    bool enviarCienciaOperacao(const QString &chNFe, const QString &cnpjEmit);
    void consultarEManifestar();
    void consultarEBaixarXML();
    QString getUltNsu();
    QString getUltNsuXml();
    bool possoConsultar();
    void consultaAlternada();
private:
    QSqlDatabase db;
    QMap<QString, QString> fiscalValues;
    QMap<QString, QString> empresaValues;
    QString cuf,cnpj;
    QString ultimo_nsu;
    QString ultNsuXml;
    QString novoUltNsuXml;

    void salvarEventoNoBanco(const QString &tipo, const EventoRetornoInfo &info, const QString &chaveNFe);
    void carregarConfigs();
    void processarHeaderDfe(const QString &bloco);
    void salvarNovoUltNsu(const QString &ultNsu);
    bool existeCienciaOperacao(const QString &chNFe);
    void salvarResumoNota(ResumoNFe resumo);
    void processarResumo(const QString &bloco);
    void processarNota(const QString &bloco);
    bool salvarEmitenteCliente(ProcNfe notaInfo);
    Emitente lerEmitenteDoXML(const QString &xmlPath);
    NotaFiscal lerNotaFiscalDoXML(const QString &xmlPath);
    bool atualizarNotaBanco(ProcNfe notaInfo);
    void salvarNovoUltNsuXml(const QString &ultnsuxml);
    void atualizarDataNsu(int option);
    void processarHeaderDfeXML(const QString &bloco);
    QList<ProdutoNota> carregarProdutosDaNFe(const QString &xml_path, qlonglong id_nf);
    bool salvarProdutosNota(const QString &xml_path, const QString &chnfe);
signals:
};

#endif // MANIFESTADORDFE_H
