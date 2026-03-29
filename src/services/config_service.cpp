#include "config_service.h"

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
