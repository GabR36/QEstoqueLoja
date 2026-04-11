#ifndef CONFIG_REPOSITORY_H
#define CONFIG_REPOSITORY_H

#include <QObject>
#include "../dto/Config_dto.h"

class Config_repository : public QObject
{
    Q_OBJECT
public:
    explicit Config_repository(QObject *parent = nullptr);
    ConfigDTO loadAll();
    bool saveAll(const ConfigDTO &config);
signals:
};

#endif // CONFIG_REPOSITORY_H
