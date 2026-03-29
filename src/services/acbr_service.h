#ifndef ACBR_SERVICE_H
#define ACBR_SERVICE_H

#include <QObject>
#include <QMap>
#include "../services/config_service.h"
#include "../util/mailmanager.h"
#include "../nota/acbrmanager.h"
#include "../util/consultacnpjmanager.h"

enum class AcbrErro {
    Nenhum,
    CampoVazio,
    ConfiguracaoInvalida,
    NaoEmitindoNf
};

class Acbr_service : public QObject
{
    Q_OBJECT

public:
    struct Resultado {
        bool ok;
        AcbrErro erro = AcbrErro::Nenhum;
        QString msg;
    };
    explicit Acbr_service(QObject *parent = nullptr);
    Acbr_service::Resultado configurar(const QString &versaoApp);
    Acbr_service::Resultado carregarConfigParaDFE();
private:
    ConfigDTO configDTO;
    ACBrNFe *nfe;
    ACBrMail *mail;
    ACBrConsultaCNPJ *cnpj;

signals:
};

#endif // ACBR_SERVICE_H
