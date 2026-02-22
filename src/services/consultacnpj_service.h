#ifndef CONSULTACNPJ_SERVICE_H
#define CONSULTACNPJ_SERVICE_H

#include <QObject>
#include "../util/consultacnpjmanager.h"
#include "../dto/Cliente_dto.h"


class ConsultaCnpj_service : public QObject
{
    Q_OBJECT
public:
    explicit ConsultaCnpj_service(QObject *parent = nullptr);
    ClienteDTO getInfo(QString cnpj);
private:
    ACBrConsultaCNPJ *cnpjManager;

signals:
};

#endif // CONSULTACNPJ_SERVICE_H
