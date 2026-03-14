#ifndef EMAIL_SERVICE_H
#define EMAIL_SERVICE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <string>
#include "config_service.h"

class Email_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        QString msg;
    };

    explicit Email_service(QObject *parent = nullptr);

    Resultado enviarEmailNFe(QString nomeCliente, QString emailCliente,
                             QString xmlPath, std::string pdfDanfe,
                             QDateTime dataVenda, QString nomeEmpresa);
private:
    Config_service confServ;
    ConfigDTO confDTO;
};

#endif // EMAIL_SERVICE_H
