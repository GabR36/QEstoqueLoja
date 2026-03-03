#include "configuracao.h"
#include "ui_configuracao.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include "util/NfUtilidades.h"
#include <QStandardPaths>
#include "util/helppage.h"
#include "dto/Config_dto.h"
#include "services/imagestorage_service.h"

Configuracao::Configuracao(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Config)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0); // começa na tab Empresa

    ui->Btn_CertficadoAcPath->setVisible(false);
    ui->Lbl_CertificadoAcPath->setVisible(false);
    ui->LEdit_CESTProd->setVisible(false);
    ui->Lbl_CestProd->setVisible(false);



    ConfigDTO configDTO;
    configService = new Config_service(this);

    configDTO = configService->carregarTudo();

    ui->Ledt_CnpjEmpresa->setText(configDTO.cnpjEmpresa);
    ui->Ledt_EmailEmpresa->setText(configDTO.emailEmpresa);
    ui->Ledt_EnderecoEmpresa->setText(configDTO.enderecoEmpresa);
    ui->Ledt_CidadeEmpresa->setText(configDTO.cidadeEmpresa);
    ui->Ledt_EstadoEmpresa->setText(configDTO.estadoEmpresa);
    ui->Ledt_NomeEmpresa->setText(configDTO.nomeEmpresa);
    ui->Ledt_TelEmpresa->setText(configDTO.telefoneEmpresa);
    ui->Ledt_LogoEmpresa->setText(configDTO.logoPathEmpresa);
    // fiscal
    ui->Ledt_NomeFantasia->setText(configDTO.nomeFantasiaEmpresa);
    ui->Ledt_NumeroEnd->setText(configDTO.numeroEmpresa);
    ui->Ledt_Bairro->setText(configDTO.bairroEmpresa);
    ui->Ledt_Cep->setText(configDTO.cepEmpresa);
    ui->CBox_Opcao->setCurrentIndex(configDTO.regimeTribFiscal);
    ui->CBox_Tpamp->setCurrentIndex(configDTO.tpAmbFiscal);
    ui->Ledit_Idcsc->setText(configDTO.idCscFiscal);
    ui->Ledit_Csc->setText(configDTO.cscFiscal);
    ui->Lbl_SchemaPath->setText(configDTO.schemaPathFiscal);
    ui->Lbl_CertificadoAcPath->setText(configDTO.certAcPathFiscal);
    ui->Lbl_CertificadoPath->setText(configDTO.certificadoPathFiscal);
    ui->Ledit_CertificadoSenha->setText(configDTO.senhaCertificadoFiscal);
    ui->Ledt_CUf->setText(configDTO.cUfFiscal);
    ui->Ledt_CMun->setText(configDTO.cMunFiscal);
    ui->Ledt_IE->setText(configDTO.iEstadFiscal);
    ui->Ledt_CnpjRespTec->setText(configDTO.cnpjRTFiscal);
    ui->Ledt_NomeRespTec->setText(configDTO.nomeRTFiscal);
    ui->Ledt_EmailRespTec->setText(configDTO.emailRTFiscal);
    ui->Ledt_FoneRespTec->setText(configDTO.foneRTFiscal);
    ui->Ledt_IdCsrtRespTec->setText(configDTO.idCSRTFiscal);
    ui->Ledt_HashCsrtRespTec->setText(configDTO.hashCSRTFiscal);
    // converter notacao americana em ptbr
    ui->Ledt_LucroEmpresa->setText(portugues.toString(configDTO.porcentLucroFinanceiro));
    ui->Ledt_debito->setText(portugues.toString(configDTO.taxaDebitoFinanceiro));
    ui->Ledt_credito->setText(portugues.toString(configDTO.taxaCreditoFinanceiro));
    ui->CheckBox_emitNf->setChecked(configDTO.emitNfFiscal);
    ui->CheckBox_usarIbs->setChecked(configDTO.usarIbsFiscal);
    ui->Ledit_NNfHomolog->setText(QString::number(configDTO.nnfHomologFiscal));
    ui->Ledit_NNfProd->setText(QString::number(configDTO.nnfProdFiscal));
    ui->Ledit_NNFProdNFe->setText(QString::number(configDTO.nnfProdNfeFiscal));
    ui->Ledit_NNfHomologNFe->setText(QString::number(configDTO.nnfHomologNfeFiscal));

    //produto
    ui->Ledit_NCMProd->setText(configDTO.ncmPadraoProduto);
    ui->Ledit_CSOSNProd->setText(configDTO.csosnPadraoProduto);
    ui->Ledit_PISProd->setText(configDTO.pisPadraoProduto);
    ui->LEdit_CESTProd->setText(configDTO.cestPadraoProduto);

    //email
    ui->Ledit_EmailNome->setText(configDTO.nomeEmail);
    ui->Ledit_EmailUsuario->setText(configDTO.usuarioEmail);
    ui->Ledit_EmailSmtp->setText(configDTO.smtpEmail);
    ui->Ledit_EmailConta->setText(configDTO.contaEmail);
    ui->Ledit_EmailSenha->setText(configDTO.senhaEmail);
    ui->Ledit_EmailPorta->setText(configDTO.portaEmail);
    ui->CheckBox_EmailSSL->setChecked(configDTO.sslEmail);
    ui->CheckBox_EmailTLS->setChecked(configDTO.tlsEmail);

    //contador
    ui->Ledit_ContadorNome->setText(configDTO.nomeContador);
    ui->Ledit_ContadorEmail->setText(configDTO.emailContador);



    // validador
    QDoubleValidator *validador = new QDoubleValidator();
    ui->Ledt_LucroEmpresa->setValidator(validador);
    ui->Ledt_credito->setValidator(validador);
    ui->Ledt_debito->setValidator(validador);

    QIntValidator *validatorUint = new QIntValidator(0, INT_MAX, this);
    ui->Ledit_NNfHomolog->setValidator(validatorUint);
    ui->Ledit_NNfProd->setValidator(validatorUint);
    ui->Ledit_CSOSNProd->setValidator(validatorUint);
    QRegularExpression ncmRegex("^\\d{0,8}$");  // até 8 dígitos
    QRegularExpression cestRegex("^\\d{0,7}$"); // até 7 dígitos
    ui->LEdit_CESTProd->setValidator(new QRegularExpressionValidator(cestRegex, this));
    ui->Ledit_NCMProd->setValidator(new QRegularExpressionValidator(ncmRegex, this));
    ui->Ledit_PISProd->setValidator(validatorUint);

    QRegularExpression sohNumeroRegex("^[0-9]*$");
    ui->Ledit_NNfHomolog->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledit_NNfProd->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledit_NNfHomologNFe->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledit_NNFProdNFe->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledit_Idcsc->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledt_CUf->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledt_IE->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledt_CMun->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledt_CnpjRespTec->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledt_FoneRespTec->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));

    ui->Ledt_NumeroEnd->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledt_Cep->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledt_TelEmpresa->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));
    ui->Ledt_CnpjEmpresa->setValidator(new QRegularExpressionValidator(sohNumeroRegex, this));

}

Configuracao::~Configuracao()
{
    delete ui;
}


void Configuracao::on_Btn_Aplicar_clicked()
{
    // QString nomeEmpresa, nomeFant, enderecoEmpresa, numeroEmpresa, bairroEmpresa, cepEmpresa, emailEmpresa,
    //     cnpjEmpresa, telEmpresa, lucro, debito, credito, cidadeEmpresa, estadoEmpresa, logoEmpresa, regimeTrib, tpAmb,
    //     idCsc, csc, pastaSchema, pastaCertificadoAc, certificado, senhaCertificado, cUf, cMun, iEstad, cnpjRT, nomeRT,
    //     emailRt, foneRt, idCSRT, hasCsrt, nnfHomolog, nnfProd, csosnPadrao, ncmPadrao, cestPadrao, pisPadrao,
    //     nnfHomologNfe, nnfProdNfe, email_nome, email_smtp, email_conta, email_usuario, email_senha, email_porta,
    //     contador_nome, contador_email;
    // bool emitNf, usarIbs, email_ssl, email_tls;

    ConfigDTO dtoInserir;

    // converter para notacao americana para armazenar no banco de dados
    dtoInserir.nomeEmpresa = ui->Ledt_NomeEmpresa->text().trimmed();
    dtoInserir.nomeFantasiaEmpresa = ui->Ledt_NomeFantasia->text().trimmed();
    dtoInserir.enderecoEmpresa = ui->Ledt_EnderecoEmpresa->text().trimmed();
    dtoInserir.numeroEmpresa = ui->Ledt_NumeroEnd->text().trimmed();
    dtoInserir.bairroEmpresa = ui->Ledt_Bairro->text().trimmed();
    dtoInserir.cepEmpresa = ui->Ledt_Cep->text().trimmed();
    dtoInserir.emailEmpresa = ui->Ledt_EmailEmpresa->text().trimmed();
    dtoInserir.cnpjEmpresa = ui->Ledt_CnpjEmpresa->text().trimmed();
    dtoInserir.telefoneEmpresa = ui->Ledt_TelEmpresa->text().trimmed();
    dtoInserir.cidadeEmpresa = ui->Ledt_CidadeEmpresa->text().trimmed();
    dtoInserir.estadoEmpresa = ui->Ledt_EstadoEmpresa->text().trimmed();
    dtoInserir.logoPathEmpresa = ui->Ledt_LogoEmpresa->text().trimmed();
    dtoInserir.porcentLucroFinanceiro = portugues.toFloat(ui->Ledt_LucroEmpresa->text());
    dtoInserir.taxaDebitoFinanceiro = portugues.toFloat(ui->Ledt_debito->text());
    dtoInserir.taxaCreditoFinanceiro = portugues.toFloat(ui->Ledt_credito->text());
    dtoInserir.regimeTribFiscal = ui->CBox_Opcao->currentIndex();
    dtoInserir.tpAmbFiscal = ui->CBox_Tpamp->currentIndex();
    dtoInserir.idCscFiscal = ui->Ledit_Idcsc->text().trimmed();
    dtoInserir.cscFiscal = ui->Ledit_Csc->text().trimmed();
    dtoInserir.schemaPathFiscal = ui->Lbl_SchemaPath->text().trimmed();
    dtoInserir.certAcPathFiscal = ui->Lbl_CertificadoAcPath->text().trimmed();
    dtoInserir.certificadoPathFiscal = ui->Lbl_CertificadoPath->text().trimmed();
    dtoInserir.senhaCertificadoFiscal = ui->Ledit_CertificadoSenha->text().trimmed();
    dtoInserir.cUfFiscal = ui->Ledt_CUf->text().trimmed();
    dtoInserir.cMunFiscal = ui->Ledt_CMun->text().trimmed();
    dtoInserir.iEstadFiscal = ui->Ledt_IE->text().trimmed();
    dtoInserir.cnpjRTFiscal = ui->Ledt_CnpjRespTec->text().trimmed();
    dtoInserir.nomeRTFiscal = ui->Ledt_NomeRespTec->text().trimmed();
    dtoInserir.emailRTFiscal = ui->Ledt_EmailRespTec->text().trimmed();
    dtoInserir.foneRTFiscal = ui->Ledt_FoneRespTec->text().trimmed();
    dtoInserir.idCSRTFiscal = ui->Ledt_IdCsrtRespTec->text().trimmed();
    dtoInserir.hashCSRTFiscal = ui->Ledt_HashCsrtRespTec->text().trimmed();
    dtoInserir.emitNfFiscal = ui->CheckBox_emitNf->isChecked();
    dtoInserir.usarIbsFiscal = ui->CheckBox_usarIbs->isChecked();
    dtoInserir.nnfHomologFiscal = ui->Ledit_NNfHomolog->text().toInt();
    dtoInserir.nnfProdFiscal = ui->Ledit_NNfProd->text().toInt();
    dtoInserir.csosnPadraoProduto = ui->Ledit_CSOSNProd->text().trimmed();
    dtoInserir.ncmPadraoProduto = ui->Ledit_NCMProd->text().trimmed();
    dtoInserir.cestPadraoProduto = ui->LEdit_CESTProd->text().trimmed();
    dtoInserir.pisPadraoProduto = ui->Ledit_PISProd->text().trimmed();
    dtoInserir.nnfProdNfeFiscal = ui->Ledit_NNFProdNFe->text().toInt();
    dtoInserir.nnfHomologNfeFiscal = ui->Ledit_NNfHomologNFe->text().toInt();
    dtoInserir.nomeEmail = ui->Ledit_EmailNome->text().trimmed();
    dtoInserir.contaEmail = ui->Ledit_EmailConta->text().trimmed();
    dtoInserir.smtpEmail = ui->Ledit_EmailSmtp->text().trimmed();
    dtoInserir.usuarioEmail = ui->Ledit_EmailUsuario->text().trimmed();
    dtoInserir.senhaEmail = ui->Ledit_EmailSenha->text().trimmed();
    dtoInserir.portaEmail = ui->Ledit_EmailPorta->text().trimmed();
    dtoInserir.sslEmail = ui->CheckBox_EmailSSL->isChecked();
    dtoInserir.tlsEmail = ui->CheckBox_EmailTLS->isChecked();
    dtoInserir.nomeContador = ui->Ledit_ContadorNome->text().trimmed();
    dtoInserir.emailContador = ui->Ledit_ContadorEmail->text().trimmed();


    QString erro = "";
    if(!configService->salvarTudo(dtoInserir, erro)){
        QMessageBox::warning(this, "Erro", erro);
        return;
    }
    emit alterouConfig();

    this->close();
}


void Configuracao::on_Btn_Cancelar_clicked()
{
    this->close();
}


void Configuracao::on_Btn_LogoEmpresa_clicked()
{
    QString caminhoOriginal = QFileDialog::getOpenFileName(
        this,
        tr("Selecionar Logo da Empresa"),
        "",
        tr("Imagens (*.png *.jpg *.jpeg *.bmp)")
        );

    if (caminhoOriginal.isEmpty())
        return;

    QString erro;
    ImageStorage_service storage;

    QString caminhoRelativo = storage.saveLogoEmpresa(caminhoOriginal, erro);

    if(caminhoRelativo.isEmpty()){
        QMessageBox::warning(this, "Erro", erro);
        return;
    }

    ui->Ledt_LogoEmpresa->setText(caminhoRelativo);
}



void Configuracao::on_Btn_schemaPath_clicked()
{
    QString pastaSelecionada = QFileDialog::getExistingDirectory(
        this,
        tr("Selecionar a Pasta dos Schemas"),
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (pastaSelecionada.isEmpty())
        return;

    ui->Lbl_SchemaPath->setText(pastaSelecionada);

}


void Configuracao::on_Btn_certificadoPath_clicked()
{
    QString caminhoOriginal = QFileDialog::getOpenFileName(
        this,
        tr("Selecionar o Certificado A1 da Empresa"),
        "",
        tr("Certificado (*.pfx)")
        );
    if (caminhoOriginal.isEmpty())
        return;
    ui->Lbl_CertificadoPath->setText(caminhoOriginal);
}

void Configuracao::on_label_35_linkActivated(const QString &link)
{
    if (link == "help") {
        HelpPage *help = new HelpPage(this, "fiscal");
        help->show();
    }
}

