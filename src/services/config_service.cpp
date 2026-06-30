#include "config_service.h"
#include <QDir>
#include "../infra/databaseconnection_service.h"

Config_service::Config_service(QObject *parent)
    : QObject{parent}
{
    m_repo = new Config_repository(this);
}

ConfigDTO Config_service::carregarTudo()
{
    return m_repo->loadAll();
}



bool Config_service::salvarTudo(const ConfigDTO &dto, QString &erro)
{
    // validações
    if(!validarFiscal(dto, erro)) return false;

    // persistência
    return m_repo->saveAll(dto);
}

bool Config_service::validarFiscal(const ConfigDTO &dto, QString &erro)
{
    if(dto.emitNfFiscal){
        if(dto.certificadoPathFiscal.isEmpty() ||
            dto.senhaCertificadoFiscal.isEmpty() ||
            dto.cscFiscal.isEmpty() ||
            dto.idCscFiscal.isEmpty()){
            erro = "Configuração fiscal incompleta: certificado, senha, CSC e ID CSC são obrigatórios.";
            return false;
        }
    }
    return true;
}


bool Config_service::verificarInfoDB(const ConfigDTO &dto, QString &erro)
{
    if(dto.driverDB == 1){ // postgre
        // Campos obrigatórios
        if (dto.ipHostDB.isEmpty()) {
            erro = "Informe o endereço do servidor.";
            return false;
        }

        if (dto.nomeDB.isEmpty()) {
            erro = "Informe o nome do banco de dados.";
            return false;
        }

        if (dto.userDB.isEmpty()) {
            erro = "Informe o usuário do banco.";
            return false;
        }

        if (dto.senhaDB.isEmpty()) {
            erro = "Informe a senha do banco.";
            return false;
        }

        if (!m_repo->testarConexaoBanco(dto, erro))
            return false;

        // Verificar pasta compartilhada
        QDir dir(dto.pathPastaPostgreDB);

        if (!dir.exists()) {
            erro = "A pasta compartilhada não existe ou não está acessível.";
            return false;
        }
    }else if(dto.driverDB == 0){ // sqlite
        QDir dir(dto.pathPastaSqliteDB);

        if (!dir.exists()) {
            erro = "A pasta do banco de dados não existe ou não está acessível.";
            return false;
        }
    }


    return true;
}

void Config_service::mudarDatabase(ConfigDTO config){
    DatabaseConnection_service::changeDatabase(config);
}

ConfigDbDTO Config_service::getConfigsDB(){
    return m_repo->getConfigsDb();
}

