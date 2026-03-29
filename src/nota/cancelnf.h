#ifndef CANCELNF_H
#define CANCELNF_H

#include <QObject>
#include "acbrmanager.h"
#include <QSqlDatabase>
#include <sstream>
#include "../dto/EventoFiscal_dto.h"


class cancelNf : public QObject
{
    Q_OBJECT
public:
    explicit cancelNf(QObject *parent = nullptr, qlonglong idnf = 0);
    QString gerarEnviar();
    QString getPath();
    EventoFiscalDTO GerarEnviarRetorno();
private:
    ACBrNFe *acbr;
    std::stringstream ini;
    QSqlDatabase db;
    QString cnpjemit, chnfe, nprot, cuf;


    void pegarDados(qlonglong idnf);
    void preencherEvento();
signals:
};

#endif // CANCELNF_H
