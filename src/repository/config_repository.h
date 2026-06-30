#ifndef CONFIG_REPOSITORY_H
#define CONFIG_REPOSITORY_H

#include <QObject>
#include "../dto/Config_dto.h"
#include "../dto/ConfigDB_dto.h"

class Config_repository : public QObject
{
    Q_OBJECT
public:
    explicit Config_repository(QObject *parent = nullptr);
    ConfigDTO loadAll();
    bool saveAll(const ConfigDTO &config);
    bool testarConexaoBanco(const ConfigDTO &dto, QString &erro);
    ConfigDbDTO getConfigsDb();
signals:
};

#endif // CONFIG_REPOSITORY_H
