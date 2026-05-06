#include "acbrmanager.h"
#include <QDebug>
#include "../infra/apppath_service.h"
#include <QStandardPaths>

std::unique_ptr<AcbrManager> AcbrManager::m_instance = nullptr;

AcbrManager::AcbrManager(QObject *parent)
    : QObject(parent)
{
    QString configLibPath = AppPath_service::nfeConfigPath();
    qDebug() << "Inicializando ACBrLibNFe...";
    m_nfe = std::make_unique<ACBrNFe>(configLibPath.toStdString(), "");
    qDebug() << "Versão da ACBrLib:" << QString::fromStdString(m_nfe->Versao());
}

AcbrManager* AcbrManager::instance() {
    if (!m_instance)
        m_instance = std::make_unique<AcbrManager>();
    return m_instance.get();
}

ACBrNFe* AcbrManager::nfe() {
    return m_nfe.get();
}
