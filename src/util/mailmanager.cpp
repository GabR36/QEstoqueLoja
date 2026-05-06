#include "mailmanager.h"
#include <QStandardPaths>
#include "../infra/apppath_service.h"

MailManager::MailManager(QObject *parent)
    : QObject(parent)
{
}

MailManager& MailManager::instance()
{
    static MailManager instance;
    return instance;
}

ACBrMail* MailManager::mail()
{
    if (!m_mail) {
        QString configLibPath = AppPath_service::mailConfigPath();

        m_mail = std::make_unique<ACBrMail>(
            configLibPath.toStdString(),
            "",
            ""
            );
    }
    return m_mail.get();
}

void MailManager::resetLib()
{
    m_mail.reset();
}
