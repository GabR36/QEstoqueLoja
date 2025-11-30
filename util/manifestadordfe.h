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

class ManifestadorDFe : public QObject
{
    Q_OBJECT
public:
    explicit ManifestadorDFe(QObject *parent = nullptr);
    void processarResumo(const QString &bloco);
    bool enviarCienciaOperacao(const QString &chNFe, const QString &cnpjEmit);
    void consultarEManifestar();
private:
    QSqlDatabase db;
    QMap<QString, QString> fiscalValues;
    QMap<QString, QString> empresaValues;
    QString cuf,cnpj;
    QString ultimo_nsu;

    void salvarEventoNoBanco(const QString &tipo, const EventoRetornoInfo &info, const QString &chaveNFe);
    void carregarConfigs();
    QString getUltNsu();
    void processarHeaderDfe(const QString &bloco);
    void salvarNovoUltNsu(const QString &ultNsu);
    bool existeCienciaOperacao(const QString &chNFe);
    void salvarResumoNota(ResumoNFe resumo);
signals:
};

#endif // MANIFESTADORDFE_H
