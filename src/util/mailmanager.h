#ifndef MAILMANAGER_H
#define MAILMANAGER_H

#include <QObject>
#include <memory>
#include "ACBrMail.h"

class MailManager : public QObject
{
    Q_OBJECT
public:
    static MailManager& instance();

    ACBrMail* mail();
    void resetLib();

private:
    explicit MailManager(QObject *parent = nullptr);
    ~MailManager() = default;

    MailManager(const MailManager&) = delete;
    MailManager& operator=(const MailManager&) = delete;

    std::unique_ptr<ACBrMail> m_mail;
};

#endif // MAILMANAGER_H
