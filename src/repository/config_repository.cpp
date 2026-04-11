#include "config_repository.h"
#include "../infra/apppath_service.h"
#include <QSettings>

Config_repository::Config_repository(QObject *parent)
    : QObject{parent}
{}

ConfigDTO Config_repository::loadAll()
{
    ConfigDTO dto;
    QSettings s(AppPath_service::configPath(), QSettings::IniFormat);

    dto.nomeEmpresa             = s.value("empresa/nome_empresa").toString();
    dto.nomeFantasiaEmpresa     = s.value("empresa/nfant_empresa").toString();
    dto.enderecoEmpresa         = s.value("empresa/endereco_empresa").toString();
    dto.numeroEmpresa           = s.value("empresa/numero_empresa").toString();
    dto.bairroEmpresa           = s.value("empresa/bairro_empresa").toString();
    dto.cepEmpresa              = s.value("empresa/cep_empresa").toString();
    dto.cidadeEmpresa           = s.value("empresa/cidade_empresa").toString();
    dto.estadoEmpresa           = s.value("empresa/estado_empresa").toString();
    dto.emailEmpresa            = s.value("empresa/email_empresa").toString();
    dto.telefoneEmpresa         = s.value("empresa/telefone_empresa").toString();
    dto.cnpjEmpresa             = s.value("empresa/cnpj_empresa").toString();
    dto.logoPathEmpresa         = s.value("empresa/caminho_logo_empresa").toString();

    dto.regimeTribFiscal        = s.value("fiscal/regime_trib").toInt();
    dto.tpAmbFiscal             = s.value("fiscal/tp_amb").toInt();
    dto.idCscFiscal             = s.value("fiscal/id_csc").toString();
    dto.cscFiscal               = s.value("fiscal/csc").toString();
    dto.schemaPathFiscal        = s.value("fiscal/caminho_schema").toString();
    dto.certAcPathFiscal        = s.value("fiscal/caminho_certac").toString();
    dto.certificadoPathFiscal   = s.value("fiscal/caminho_certificado").toString();
    dto.senhaCertificadoFiscal  = s.value("fiscal/senha_certificado").toString();
    dto.cUfFiscal               = s.value("fiscal/cuf").toString();
    dto.cMunFiscal              = s.value("fiscal/cmun").toString();
    dto.iEstadFiscal            = s.value("fiscal/iest").toString();
    dto.cnpjRTFiscal            = s.value("fiscal/cnpj_rt").toString();
    dto.nomeRTFiscal            = s.value("fiscal/nome_rt").toString();
    dto.emailRTFiscal           = s.value("fiscal/email_rt").toString();
    dto.foneRTFiscal            = s.value("fiscal/fone_rt").toString();
    dto.idCSRTFiscal            = s.value("fiscal/id_csrt").toString();
    dto.hashCSRTFiscal          = s.value("fiscal/hash_csrt").toString();
    dto.emitNfFiscal            = s.value("fiscal/emit_nf").toString() == "1";
    dto.usarIbsFiscal           = s.value("fiscal/usar_ibs").toString() == "1";
    dto.nnfHomologFiscal        = s.value("fiscal/nnf_homolog").toInt();
    dto.nnfProdFiscal           = s.value("fiscal/nnf_prod").toInt();
    dto.nnfHomologNfeFiscal     = s.value("fiscal/nnf_homolog_nfe").toInt();
    dto.nnfProdNfeFiscal        = s.value("fiscal/nnf_prod_nfe").toInt();

    dto.ncmPadraoProduto        = s.value("produto/ncm_padrao").toString();
    dto.csosnPadraoProduto      = s.value("produto/csosn_padrao").toString();
    dto.pisPadraoProduto        = s.value("produto/pis_padrao").toString();
    dto.cestPadraoProduto       = s.value("produto/cest_padrao").toString();

    dto.porcentLucroFinanceiro  = s.value("financeiro/porcent_lucro").toDouble();
    dto.taxaDebitoFinanceiro    = s.value("financeiro/taxa_debito").toDouble();
    dto.taxaCreditoFinanceiro   = s.value("financeiro/taxa_credito").toDouble();

    dto.nomeEmail               = s.value("email/email_nome").toString();
    dto.smtpEmail               = s.value("email/email_smtp").toString();
    dto.contaEmail              = s.value("email/email_conta").toString();
    dto.usuarioEmail            = s.value("email/email_usuario").toString();
    dto.senhaEmail              = s.value("email/email_senha").toString();
    dto.portaEmail              = s.value("email/email_porta").toString();
    dto.sslEmail                = s.value("email/email_ssl").toString() == "1";
    dto.tlsEmail                = s.value("email/email_tls").toString() == "1";

    dto.nomeContador            = s.value("contador/contador_nome").toString();
    dto.emailContador           = s.value("contador/contador_email").toString();

    return dto;
}

bool Config_repository::saveAll(const ConfigDTO &dto)
{
    QSettings s(AppPath_service::configPath(), QSettings::IniFormat);

    s.setValue("empresa/nome_empresa",          dto.nomeEmpresa);
    s.setValue("empresa/nfant_empresa",         dto.nomeFantasiaEmpresa);
    s.setValue("empresa/endereco_empresa",      dto.enderecoEmpresa);
    s.setValue("empresa/numero_empresa",        dto.numeroEmpresa);
    s.setValue("empresa/bairro_empresa",        dto.bairroEmpresa);
    s.setValue("empresa/cep_empresa",           dto.cepEmpresa);
    s.setValue("empresa/cidade_empresa",        dto.cidadeEmpresa);
    s.setValue("empresa/estado_empresa",        dto.estadoEmpresa);
    s.setValue("empresa/email_empresa",         dto.emailEmpresa);
    s.setValue("empresa/telefone_empresa",      dto.telefoneEmpresa);
    s.setValue("empresa/cnpj_empresa",          dto.cnpjEmpresa);
    s.setValue("empresa/caminho_logo_empresa",  dto.logoPathEmpresa);

    s.setValue("fiscal/regime_trib",            dto.regimeTribFiscal);
    s.setValue("fiscal/tp_amb",                 dto.tpAmbFiscal);
    s.setValue("fiscal/id_csc",                 dto.idCscFiscal);
    s.setValue("fiscal/csc",                    dto.cscFiscal);
    s.setValue("fiscal/caminho_schema",         dto.schemaPathFiscal);
    s.setValue("fiscal/caminho_certac",         dto.certAcPathFiscal);
    s.setValue("fiscal/caminho_certificado",    dto.certificadoPathFiscal);
    s.setValue("fiscal/senha_certificado",      dto.senhaCertificadoFiscal);
    s.setValue("fiscal/cuf",                    dto.cUfFiscal);
    s.setValue("fiscal/cmun",                   dto.cMunFiscal);
    s.setValue("fiscal/iest",                   dto.iEstadFiscal);
    s.setValue("fiscal/cnpj_rt",                dto.cnpjRTFiscal);
    s.setValue("fiscal/nome_rt",                dto.nomeRTFiscal);
    s.setValue("fiscal/email_rt",               dto.emailRTFiscal);
    s.setValue("fiscal/fone_rt",                dto.foneRTFiscal);
    s.setValue("fiscal/id_csrt",                dto.idCSRTFiscal);
    s.setValue("fiscal/hash_csrt",              dto.hashCSRTFiscal);
    s.setValue("fiscal/emit_nf",                dto.emitNfFiscal  ? "1" : "0");
    s.setValue("fiscal/usar_ibs",               dto.usarIbsFiscal ? "1" : "0");
    s.setValue("fiscal/nnf_homolog",            dto.nnfHomologFiscal);
    s.setValue("fiscal/nnf_prod",               dto.nnfProdFiscal);
    s.setValue("fiscal/nnf_homolog_nfe",        dto.nnfHomologNfeFiscal);
    s.setValue("fiscal/nnf_prod_nfe",           dto.nnfProdNfeFiscal);

    s.setValue("produto/ncm_padrao",            dto.ncmPadraoProduto);
    s.setValue("produto/csosn_padrao",          dto.csosnPadraoProduto);
    s.setValue("produto/pis_padrao",            dto.pisPadraoProduto);
    s.setValue("produto/cest_padrao",           dto.cestPadraoProduto);

    s.setValue("financeiro/porcent_lucro",      dto.porcentLucroFinanceiro);
    s.setValue("financeiro/taxa_debito",        dto.taxaDebitoFinanceiro);
    s.setValue("financeiro/taxa_credito",       dto.taxaCreditoFinanceiro);

    s.setValue("email/email_nome",              dto.nomeEmail);
    s.setValue("email/email_smtp",              dto.smtpEmail);
    s.setValue("email/email_conta",             dto.contaEmail);
    s.setValue("email/email_usuario",           dto.usuarioEmail);
    s.setValue("email/email_senha",             dto.senhaEmail);
    s.setValue("email/email_porta",             dto.portaEmail);
    s.setValue("email/email_ssl",               dto.sslEmail ? "1" : "0");
    s.setValue("email/email_tls",               dto.tlsEmail ? "1" : "0");

    s.setValue("contador/contador_nome",        dto.nomeContador);
    s.setValue("contador/contador_email",       dto.emailContador);

    s.sync();
    return s.status() == QSettings::NoError;
}
