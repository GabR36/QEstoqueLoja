#ifndef CONFIG_SERVICE_H
#define CONFIG_SERVICE_H

#include <QObject>
#include "../dto/Config_dto.h"
#include "../repository/config_repository.h"

class Config_service : public QObject
{
    Q_OBJECT
public:
    explicit Config_service(QObject *parent = nullptr);
    // Load
    ConfigDTO carregarTudo();

    // Save
    bool salvarTudo(const ConfigDTO &dto, QString &erro);

private:
    Config_repository *m_repo;

    bool validarFiscal(const ConfigDTO &dto, QString &erro);
    bool validarEmail(const ConfigDTO &dto, QString &erro);
signals:
};

#endif // CONFIG_SERVICE_H
