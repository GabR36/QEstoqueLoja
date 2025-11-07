#ifndef CONSULTACNPJMANAGER_H
#define CONSULTACNPJMANAGER_H
#include "ACBrConsultaCNPJ.h"
#include <memory>
#include <QObject>

class ConsultaCnpjManager : public QObject
{
    Q_OBJECT
public:
    explicit ConsultaCnpjManager(QObject *parent = nullptr);
    static ConsultaCnpjManager* instance();
    ACBrConsultaCNPJ *cnpj();
    void resetLib();
private:
    static std::unique_ptr<ConsultaCnpjManager> m_instance;
    std::unique_ptr<ACBrConsultaCNPJ> m_cnpj;

signals:
};

#endif // CONSULTACNPJMANAGER_H
