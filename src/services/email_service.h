#ifndef EMAIL_SERVICE_H
#define EMAIL_SERVICE_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QDateTime>
#include <QMap>
#include <string>
#include "config_service.h"
#include "notafiscal_service.h"
#include "eventofiscal_service.h"

class Email_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        QString msg;
    };

    struct ContagemContador {
        QMap<QString, int> notasPorFinalidade;
        QMap<QString, int> eventosPorTipo;
    };

    explicit Email_service(QObject *parent = nullptr);

    Resultado enviarEmailNFe(QString nomeCliente, QString emailCliente,
                             QString xmlPath, std::string pdfDanfe,
                             QDateTime dataVenda, QString nomeEmpresa);

    ContagemContador buscarContagemContador(QDateTime dtIni, QDateTime dtFim);
    Resultado        exportarEEnviarEmailContador(QDateTime dtIni, QDateTime dtFim);

private:
    Config_service       confServ;
    ConfigDTO            confDTO;
    NotaFiscal_service   notaServ;
    EventoFiscal_service eventoServ;

    static QString footer();
};

#endif // EMAIL_SERVICE_H
