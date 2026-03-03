#include "config_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>

Config_repository::Config_repository(QObject *parent)
    : QObject{parent}
{
    m_db = DatabaseConnection_service::db();
}

ConfigDTO Config_repository::loadAll()
{
    ConfigDTO dto;

    if(!DatabaseConnection_service::open()){
        qDebug() << "Erro ao abrir banco";
        return dto;
    }

    QSqlQuery q(m_db);
    q.exec("SELECT key, value FROM config");

    QMap<QString, QString> map;
    while(q.next()){
        map[q.value(0).toString()] = q.value(1).toString();
    }

    m_db.close();

    dto.nomeEmpresa = map["nome_empresa"];
    dto.nomeFantasiaEmpresa = map["nfant_empresa"];
    dto.enderecoEmpresa = map["endereco_empresa"];
    dto.numeroEmpresa = map["numero_empresa"];
    dto.bairroEmpresa = map["bairro_empresa"];
    dto.cepEmpresa  = map["cep_empresa"];
    dto.cidadeEmpresa = map["cidade_empresa"];
    dto.estadoEmpresa = map["estado_empresa"];
    dto.emailEmpresa = map["email_empresa"];
    dto.telefoneEmpresa = map["telefone_empresa"];
    dto.cnpjEmpresa = map["cnpj_empresa"];
    dto.logoPathEmpresa = map["caminho_logo_empresa"];

    dto.regimeTribFiscal= map["regime_trib"].toInt();
    dto.tpAmbFiscal = map["tp_amb"].toInt();
    dto.idCscFiscal = map["id_csc"];
    dto.cscFiscal = map["csc"];
    dto.schemaPathFiscal = map["caminho_schema"];
    dto.certAcPathFiscal = map["caminho_certac"];
    dto.certificadoPathFiscal = map["caminho_certificado"];
    dto.senhaCertificadoFiscal = map["senha_certificado"];
    dto.cUfFiscal = map["cuf"];
    dto.cMunFiscal = map["cmun"];
    dto.iEstadFiscal = map["iest"];

    dto.cnpjRTFiscal = map["cnpj_rt"];
    dto.nomeRTFiscal = map["nome_rt"];
    dto.emailRTFiscal = map["email_rt"];
    dto.foneRTFiscal = map["fone_rt"];
    dto.idCSRTFiscal = map["id_csrt"];
    dto.hashCSRTFiscal = map["hash_csrt"];

    dto.emitNfFiscal = map["emit_nf"] == "1";
    dto.usarIbsFiscal = map["usar_ibs"] == "1";

    dto.nnfHomologFiscal = map["nnf_homolog"].toInt();
    dto.nnfProdFiscal = map["nnf_prod"].toInt();
    dto.nnfHomologNfeFiscal = map["nnf_homolog_nfe"].toInt();
    dto.nnfProdNfeFiscal = map["nnf_prod_nfe"].toInt();

    dto.ncmPadraoProduto = map["ncm_padrao"];
    dto.csosnPadraoProduto = map["csosn_padrao"];
    dto.pisPadraoProduto = map["pis_padrao"];
    dto.cestPadraoProduto = map["cest_padrao"];

    dto.porcentLucroFinanceiro = map["porcent_lucro"].toDouble();
    dto.taxaDebitoFinanceiro = map["taxa_debito"].toDouble();
    dto.taxaCreditoFinanceiro = map["taxa_credito"].toDouble();

    dto.nomeEmail = map["email_nome"];
    dto.smtpEmail = map["email_smtp"];
    dto.contaEmail = map["email_conta"];
    dto.usuarioEmail = map["email_usuario"];
    dto.senhaEmail = map["email_senha"];
    dto.portaEmail = map["email_porta"];
    dto.sslEmail  = map["email_ssl"] == "1";
    dto.tlsEmail = map["email_tls"] == "1";

    dto.nomeContador = map["contador_nome"];
    dto.emailContador = map["contador_email"];

    return dto;
}

bool Config_repository::saveAll(const ConfigDTO &dto)
{
    QMap<QString, QString> map;

    map["nome_empresa"] = dto.nomeEmpresa;
    map["nfant_empresa"] = dto.nomeFantasiaEmpresa;
    map["endereco_empresa"] = dto.enderecoEmpresa;
    map["numero_empresa"] = dto.numeroEmpresa;
    map["bairro_empresa"] = dto.bairroEmpresa;
    map["cep_empresa"] = dto.cepEmpresa;
    map["cidade_empresa"] = dto.cidadeEmpresa;
    map["estado_empresa"] = dto.estadoEmpresa;
    map["email_empresa"] = dto.emailEmpresa;
    map["telefone_empresa"] = dto.telefoneEmpresa;
    map["cnpj_empresa"] = dto.cnpjEmpresa;
    map["caminho_logo_empresa"] = dto.logoPathEmpresa;


    map["regime_trib"] = QString::number(dto.regimeTribFiscal);
    map["tp_amb"] = dto.tpAmbFiscal ? "1" : "0";
    map["id_csc"] = dto.idCscFiscal;
    map["csc"] = dto.cscFiscal;
    map["caminho_schema"] = dto.schemaPathFiscal;
    map["caminho_certac"] = dto.certificadoPathFiscal;
    map["caminho_certificado"] = dto.certificadoPathFiscal;
    map["senha_certificado"] = dto.senhaCertificadoFiscal;
    map["cuf"] = dto.cUfFiscal;
    map["cmun"] = dto.cMunFiscal;
    map["iest"] = dto.iEstadFiscal;

    map["cnpj_rt"] = dto.cnpjRTFiscal;
    map["nome_rt"] = dto.nomeRTFiscal;
    map["email_rt"] = dto.emailRTFiscal;
    map["fone_rt"] = dto.foneRTFiscal;
    map["id_csrt"] = dto.idCSRTFiscal;
    map["hash_csrt"] = dto.hashCSRTFiscal;
    map["emit_nf"] = dto.emitNfFiscal ? "1" : "0";
    map["usar_ibs"] = dto.usarIbsFiscal ? "1" : "0";
    map["nnf_homolog"] = QString::number(dto.nnfHomologFiscal);
    map["nnf_prod"] = QString::number(dto.nnfProdFiscal);
    map["nnf_homolog_nfe"] = QString::number(dto.nnfHomologNfeFiscal);
    map["nnf_prod_nfe"] = QString::number(dto.nnfProdNfeFiscal);

    map["ncm_padrao"] = dto.ncmPadraoProduto;
    map["csosn_padrao"] = dto.csosnPadraoProduto;
    map["pis_padrao"] = dto.pisPadraoProduto;
    map["cest_padrao"] = dto.cestPadraoProduto;

    map["porcent_lucro"] = QString::number(dto.porcentLucroFinanceiro);
    map["taxa_debito"] = QString::number(dto.taxaDebitoFinanceiro);
    map["taxa_credito"] = QString::number(dto.taxaCreditoFinanceiro);

    map["email_nome"] = dto.nomeEmail;
    map["email_smtp"] = dto.smtpEmail;
    map["email_conta"] = dto.contaEmail;
    map["email_usuario"] = dto.usuarioEmail;
    map["email_senha"] = dto.senhaEmail;
    map["email_porta"] = dto.portaEmail;
    map["email_ssl"] = dto.sslEmail ? "1" : "0";
    map["email_tls"] = dto.tlsEmail ? "1" : "0";

    map["contador_nome"] = dto.nomeContador;
    map["contador_email"] = dto.emailContador;


    if(!DatabaseConnection_service::open()){
        qDebug() << "Erro ao abrir banco";
        return false;
    }

    QSqlQuery q(m_db);
    q.prepare("UPDATE config SET value = :value WHERE key = :key");

    for(auto it = map.begin(); it != map.end(); ++it){
        q.bindValue(":value", it.value());
        q.bindValue(":key", it.key());
        if(!q.exec()){
            qDebug() << "Erro update:" << it.key() << q.lastError().text();
        }
    }

    m_db.close();
    return true;
}
