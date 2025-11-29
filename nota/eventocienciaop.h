#ifndef EVENTOCIENCIAOP_H
#define EVENTOCIENCIAOP_H

#include <QObject>
#include "../nota/acbrmanager.h"
#include <sstream>
#include <QMap>

struct EventoRetornoInfo {
    QString cStat;
    QString xMotivo;
    QString nProt;
    QString xmlPath;
    int idLote = 1;
};

class EventoCienciaOP : public QObject
{
    Q_OBJECT

public:
    explicit EventoCienciaOP(QObject *parent = nullptr, QString chnfe = "");

    QString gerarEnviar();
    QString getCampo(const QString &texto, const QString &campo);
    EventoRetornoInfo gerarEnviarRetorno();
private:
    std::stringstream ini;
    QMap<QString, QString> fiscalValues;
    QMap<QString, QString> empresaValues;
    QString chnfe_global;
    ACBrNFe *acbr;



    void preencherEvento();
signals:
};

#endif // EVENTOCIENCIAOP_H
