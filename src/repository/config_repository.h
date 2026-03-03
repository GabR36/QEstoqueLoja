#ifndef CONFIG_REPOSITORY_H
#define CONFIG_REPOSITORY_H

#include <QObject>
#include "../dto/Config_dto.h"
#include <QSqlDatabase>

class Config_repository : public QObject
{
    Q_OBJECT
public:
    explicit Config_repository(QObject *parent = nullptr);
    ConfigDTO loadAll();
    bool saveAll(const ConfigDTO &config);

    // bool updatePartial(const QMap<QString, QString> &diff);

    // bool exists(const QString &key);
    // bool insert(const QString &key, const QString &value);
private:
    QSqlDatabase m_db;
signals:

};

#endif // CONFIG_REPOSITORY_H
