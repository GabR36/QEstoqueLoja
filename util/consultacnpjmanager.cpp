#include "consultacnpjmanager.h"
#include <QStandardPaths>
#include <QDebug>

std::unique_ptr<ConsultaCnpjManager> ConsultaCnpjManager::m_instance = nullptr;

ConsultaCnpjManager::ConsultaCnpjManager(QObject *parent)
    : QObject{parent}
{
    // QString configLibPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
    // + "/acbrconsultacnpj_config.ini";
    // qDebug() << "Inicializando ACBrLibNFe...";
    // m_cnpj = std::make_unique<ACBrConsultaCNPJ>(configLibPath.toStdString(), "");
    // qDebug() << "Versão da ACBrLib:" << QString::fromStdString(m_cnpj->Versao());

}
ConsultaCnpjManager* ConsultaCnpjManager::instance(){
    if (!m_instance)
        m_instance = std::make_unique<ConsultaCnpjManager>();
    return m_instance.get();
}


ACBrConsultaCNPJ* ConsultaCnpjManager::cnpj() {
    if (!m_cnpj) {
        QString configLibPath =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + "/acbrconsultacnpj_config.ini";
        m_cnpj = std::make_unique<ACBrConsultaCNPJ>(configLibPath.toStdString(), "");
    }
    return m_cnpj.get();
}

void ConsultaCnpjManager::resetLib()
{
    m_cnpj.reset(); // destrói a instância atual com segurança
}

