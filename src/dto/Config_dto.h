#ifndef CONFIG_DTO_H
#define CONFIG_DTO_H

#include <QString>

struct ConfigDTO {

    // Empresa
    QString nomeEmpresa;
    QString nomeFantasiaEmpresa;
    QString enderecoEmpresa;
    QString numeroEmpresa;
    QString bairroEmpresa;
    QString cepEmpresa;
    QString cidadeEmpresa;
    QString estadoEmpresa;
    QString emailEmpresa;
    QString telefoneEmpresa;
    QString cnpjEmpresa;
    QString logoPathEmpresa;

    // Fiscal
    int regimeTribFiscal;
    int tpAmbFiscal;
    QString idCscFiscal;
    QString cscFiscal;
    QString schemaPathFiscal;
    QString certAcPathFiscal;
    QString certificadoPathFiscal;
    QString senhaCertificadoFiscal;
    QString cUfFiscal;
    QString cMunFiscal;
    QString iEstadFiscal;

    // Responsável técnico
    QString cnpjRTFiscal;
    QString nomeRTFiscal;
    QString emailRTFiscal;
    QString foneRTFiscal;
    QString idCSRTFiscal;
    QString hashCSRTFiscal;

    // Financeiro
    double porcentLucroFinanceiro;
    double taxaDebitoFinanceiro;
    double taxaCreditoFinanceiro;

    // Flags
    bool emitNfFiscal;
    bool usarIbsFiscal;

    // Numeração NF
    int nnfHomologFiscal;
    int nnfProdFiscal;
    int nnfProdNfeFiscal;
    int nnfHomologNfeFiscal;

    // Produto padrão
    QString ncmPadraoProduto;
    QString csosnPadraoProduto;
    QString pisPadraoProduto;
    QString cestPadraoProduto;

    // Email
    QString nomeEmail;
    QString smtpEmail;
    QString contaEmail;
    QString usuarioEmail;
    QString senhaEmail;
    QString portaEmail;
    bool sslEmail;
    bool tlsEmail;

    // Contador
    QString nomeContador;
    QString emailContador;
};

#endif // CONFIG_DTO_H
