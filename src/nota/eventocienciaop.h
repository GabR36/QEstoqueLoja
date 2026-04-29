#ifndef EVENTOCIENCIAOP_H
#define EVENTOCIENCIAOP_H

#include <QObject>
#include "../nota/acbrmanager.h"
#include <sstream>
#include <QMap>
#include "../services/config_service.h"
#include "../dto/EventoFiscal_dto.h"

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
    EventoFiscalDTO gerarEnviarRetorno();
    void setRetornoForcado(QString &retorno);
private:
    std::stringstream ini;
    QMap<QString, QString> fiscalValues;
    QMap<QString, QString> empresaValues;
    QString chnfe_global;
    ACBrNFe *acbr;
    ConfigDTO configDTO;
    QString retornoForcado = "";


    void preencherEvento();
signals:
};

#endif // EVENTOCIENCIAOP_H
