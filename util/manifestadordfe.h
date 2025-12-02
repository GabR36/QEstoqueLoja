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
    int nnf = 0;
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



private:
    QSqlDatabase db;
    QMap<QString, QString> fiscalValues;
    QMap<QString, QString> empresaValues;
    QString cuf,cnpj;
    QString ultimo_nsu;
    QString ultNsuXml;
    int novoUltNsuXml;

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
    void salvarNovoUltNsuXml(int ultnsuxml);
    void atualizarDataNsu(int option);
signals:
};

#endif // MANIFESTADORDFE_H
