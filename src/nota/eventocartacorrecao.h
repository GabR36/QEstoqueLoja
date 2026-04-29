#ifndef EVENTOCARTACORRECAO_H
#define EVENTOCARTACORRECAO_H

#include <QObject>
#include "../services/config_service.h"
#include "../dto/EventoFiscal_dto.h"
#include "../nota/acbrmanager.h"
#include <sstream>

class EventoCartaCorrecao : public QObject
{
    Q_OBJECT
public:
    explicit EventoCartaCorrecao(QObject *parent = nullptr, QString chnfe = "", int nSeq1 = 1, QString correcao = "");
    void setRetornoForcado(QString &retorno);
    EventoFiscalDTO gerarEnviarRetorno();
private:
    Config_service confServ;
    ConfigDTO confDTO;
    QString chnfe_global, correcao_global;
    int nseq_global;
    ACBrNFe *acbr;
    std::stringstream ini;
    QString retornoForcado = "";
    std::string tpevento;

    void preencherEvento();
    QString getCampo(const QString &texto, const QString &campo);
    QString getCampoSecao(const QString &texto, const QString &secao, const QString &campo);
signals:
};

#endif // EVENTOCARTACORRECAO_H
