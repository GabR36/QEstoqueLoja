#ifndef MANIFESTADORDFE_H
#define MANIFESTADORDFE_H

#include <QObject>
#include <sstream>
#include <QMap>
#include <QSqlDatabase>
#include "../nota/eventocienciaop.h"


// typedef struct {
//     QString cStat;
//     QString xMotivo;
//     QString nProt;
//     QString xmlPath;
//     int idLote = 1;
// } EventoRetornoInfo;

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

    void salvarEventoNoBanco(const QString &tipo, const EventoRetornoInfo &info, const QString &chaveNFe);
    void carregarConfigs();
signals:
};

#endif // MANIFESTADORDFE_H
