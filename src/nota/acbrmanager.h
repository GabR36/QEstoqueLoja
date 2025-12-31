#ifndef ACBRMANAGER_H
#define ACBRMANAGER_H

#include <QObject>
#include "ACBrNFe.h"
#include <memory>
#include <QObject>

class AcbrManager : public QObject {
    Q_OBJECT
public:
    explicit AcbrManager(QObject *parent = nullptr);
    static AcbrManager* instance();          // acesso global
    ACBrNFe* nfe();
private:
    static std::unique_ptr<AcbrManager> m_instance;
    std::unique_ptr<ACBrNFe> m_nfe;
};

#endif // ACBRMANAGER_H
