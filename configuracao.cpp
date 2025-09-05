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

Configuracao::Configuracao(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Config)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0); // começa na tab Empresa



    // colocar os valores do banco de dados nos line edits
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados.";
    }
    QSqlQuery query;

    QMap<QString, QString> configValues;
    query.exec("SELECT key, value FROM config");
    while (query.next()) {
        QString key = query.value(0).toString();
        QString value = query.value(1).toString();
        configValues[key] = value;
    }

    ui->Ledt_CnpjEmpresa->setText(configValues.value("cnpj_empresa", ""));
    ui->Ledt_EmailEmpresa->setText(configValues.value("email_empresa", ""));
    ui->Ledt_EnderecoEmpresa->setText(configValues.value("endereco_empresa", ""));
    ui->Ledt_CidadeEmpresa->setText(configValues.value("cidade_empresa", ""));
    ui->Ledt_EstadoEmpresa->setText(configValues.value("estado_empresa", ""));
    ui->Ledt_NomeEmpresa->setText(configValues.value("nome_empresa", ""));
    ui->Ledt_TelEmpresa->setText(configValues.value("telefone_empresa", ""));
    ui->Ledt_LogoEmpresa->setText(configValues.value("caminho_logo_empresa", ""));
    // fiscal
    ui->Ledt_NomeFantasia->setText(configValues.value("nfant_empresa", ""));
    ui->Ledt_NumeroEnd->setText(configValues.value("numero_empresa", ""));
    ui->Ledt_Bairro->setText(configValues.value("bairro_empresa", ""));
    ui->Ledt_Cep->setText(configValues.value("cep_empresa", ""));
    ui->CBox_Opcao->setCurrentIndex(configValues.value("regime_trib", "").toInt());
    ui->CBox_Tpamp->setCurrentIndex(configValues.value("tp_amb", "").toInt());
    ui->Ledit_Idcsc->setText(configValues.value("id_csc", ""));
    ui->Ledit_Csc->setText(configValues.value("csc", ""));
    ui->Lbl_SchemaPath->setText(configValues.value("caminho_schema", ""));
    ui->Lbl_CertificadoAcPath->setText(configValues.value("caminho_certac", ""));
    ui->Lbl_CertificadoPath->setText(configValues.value("caminho_certificado", ""));
    ui->Ledit_CertificadoSenha->setText(configValues.value("senha_certificado", ""));
    ui->Ledt_CUf->setText(configValues.value("cuf", ""));
    ui->Ledt_CMun->setText(configValues.value("cmun", ""));
    ui->Ledt_IE->setText(configValues.value("iest", ""));
    ui->Ledt_CnpjRespTec->setText(configValues.value("cnpj_rt", ""));
    ui->Ledt_NomeRespTec->setText(configValues.value("nome_rt", ""));
    ui->Ledt_EmailRespTec->setText(configValues.value("email_rt", ""));
    ui->Ledt_FoneRespTec->setText(configValues.value("fone_rt", ""));
    ui->Ledt_IdCsrtRespTec->setText(configValues.value("id_csrt", ""));
    ui->Ledt_HashCsrtRespTec->setText(configValues.value("hash_csrt", ""));
    // converter notacao americana em ptbr
    ui->Ledt_LucroEmpresa->setText(portugues.toString(configValues.value("porcent_lucro", "").toFloat()));
    ui->Ledt_debito->setText(portugues.toString(configValues.value("taxa_debito", "").toFloat()));
    ui->Ledt_credito->setText(portugues.toString(configValues.value("taxa_credito", "").toFloat()));
    ui->CheckBox_emitNf->setChecked(configValues.value("emit_nf") == "1");
    ui->Ledit_NNfHomolog->setText(configValues.value("nnf_homolog", ""));
    ui->Ledit_NNfProd->setText(configValues.value("nnf_prod", ""));
    ui->Ledit_NNFProdNFe->setText(configValues.value("nnf_prod_nfe", ""));
    ui->Ledit_NNfHomologNFe->setText(configValues.value("nnf_homolog_nfe", ""));

    //produto
    ui->Ledit_NCMProd->setText(configValues.value("ncm_padrao", ""));
    ui->Ledit_CSOSNProd->setText(configValues.value("csosn_padrao", ""));
    ui->Ledit_PISProd->setText(configValues.value("pis_padrao", ""));
    ui->LEdit_CESTProd->setText(configValues.value("cest_padrao", ""));






    db.close();

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
    QString nomeEmpresa, nomeFant, enderecoEmpresa, numeroEmpresa, bairroEmpresa, cepEmpresa, emailEmpresa,
        cnpjEmpresa, telEmpresa, lucro, debito, credito, cidadeEmpresa, estadoEmpresa, logoEmpresa, regimeTrib, tpAmb,
        idCsc, csc, pastaSchema, pastaCertificadoAc, certificado, senhaCertificado, cUf, cMun, iEstad, cnpjRT, nomeRT,
        emailRt, foneRt, idCSRT, hasCsrt, nnfHomolog, nnfProd, csosnPadrao, ncmPadrao, cestPadrao, pisPadrao,
        nnfHomologNfe, nnfProdNfe;
    bool emitNf;
    // converter para notacao americana para armazenar no banco de dados
    nomeEmpresa = ui->Ledt_NomeEmpresa->text().trimmed();
    nomeFant = ui->Ledt_NomeFantasia->text().trimmed();
    enderecoEmpresa = ui->Ledt_EnderecoEmpresa->text().trimmed();
    numeroEmpresa = ui->Ledt_NumeroEnd->text().trimmed();
    bairroEmpresa = ui->Ledt_Bairro->text().trimmed();
    cepEmpresa = ui->Ledt_Cep->text().trimmed();
    emailEmpresa = ui->Ledt_EmailEmpresa->text().trimmed();
    cnpjEmpresa = ui->Ledt_CnpjEmpresa->text().trimmed();
    telEmpresa = ui->Ledt_TelEmpresa->text().trimmed();
    cidadeEmpresa = ui->Ledt_CidadeEmpresa->text().trimmed();
    estadoEmpresa = ui->Ledt_EstadoEmpresa->text().trimmed();
    logoEmpresa = ui->Ledt_LogoEmpresa->text().trimmed();
    lucro = QString::number(portugues.toFloat(ui->Ledt_LucroEmpresa->text())).trimmed();
    debito = QString::number(portugues.toFloat(ui->Ledt_debito->text())).trimmed();
    credito = QString::number(portugues.toFloat(ui->Ledt_credito->text())).trimmed();
    regimeTrib = QString::number(ui->CBox_Opcao->currentIndex()).trimmed();
    tpAmb = QString::number(ui->CBox_Tpamp->currentIndex()).trimmed();
    idCsc = ui->Ledit_Idcsc->text().trimmed();
    csc = ui->Ledit_Csc->text().trimmed();
    pastaSchema = ui->Lbl_SchemaPath->text().trimmed();
    pastaCertificadoAc = ui->Lbl_CertificadoAcPath->text().trimmed();
    certificado = ui->Lbl_CertificadoPath->text().trimmed();
    senhaCertificado = ui->Ledit_CertificadoSenha->text().trimmed();
    cUf = ui->Ledt_CUf->text().trimmed();
    cMun = ui->Ledt_CMun->text().trimmed();
    iEstad = ui->Ledt_IE->text().trimmed();
    cnpjRT = ui->Ledt_CnpjRespTec->text().trimmed();
    nomeRT = ui->Ledt_NomeRespTec->text().trimmed();
    emailRt = ui->Ledt_EmailRespTec->text().trimmed();
    foneRt = ui->Ledt_FoneRespTec->text().trimmed();
    idCSRT = ui->Ledt_IdCsrtRespTec->text().trimmed();
    hasCsrt = ui->Ledt_HashCsrtRespTec->text().trimmed();
    emitNf = ui->CheckBox_emitNf->isChecked();
    nnfHomolog = ui->Ledit_NNfHomolog->text().trimmed();
    nnfProd = ui->Ledit_NNfProd->text().trimmed();
    csosnPadrao = ui->Ledit_CSOSNProd->text().trimmed();
    ncmPadrao = ui->Ledit_NCMProd->text().trimmed();
    cestPadrao = ui->LEdit_CESTProd->text().trimmed();
    pisPadrao = ui->Ledit_PISProd->text().trimmed();
    nnfProdNfe = ui->Ledit_NNFProdNFe->text().trimmed();
    nnfHomologNfe = ui->Ledit_NNfHomologNFe->text().trimmed();

    if(emitNf == true && (ui->Lbl_CertificadoPath->text().isEmpty() ||
        senhaCertificado.isEmpty() || csc.isEmpty() || idCsc.isEmpty())){
        QMessageBox::warning(this, "Erro", "Se for emitir notas fiscais deve preencher "
                                           "todos os dados");
        return;

    }


    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados.";
    }
    QSqlQuery query;

    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", nomeEmpresa);
    query.bindValue(":key", "nome_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", enderecoEmpresa);
    query.bindValue(":key", "endereco_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", cnpjEmpresa);
    query.bindValue(":key", "cnpj_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", telEmpresa);
    query.bindValue(":key", "telefone_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", lucro);
    query.bindValue(":key", "porcent_lucro");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", emailEmpresa);
    query.bindValue(":key", "email_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", debito);
    query.bindValue(":key", "taxa_debito");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", credito);
    query.bindValue(":key", "taxa_credito");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", cidadeEmpresa);
    query.bindValue(":key", "cidade_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", estadoEmpresa);
    query.bindValue(":key", "estado_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", logoEmpresa);
    query.bindValue(":key", "caminho_logo_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", nomeFant);
    query.bindValue(":key", "nfant_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", numeroEmpresa);
    query.bindValue(":key", "numero_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", bairroEmpresa);
    query.bindValue(":key", "bairro_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", bairroEmpresa);
    query.bindValue(":key", "bairro_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", cepEmpresa);
    query.bindValue(":key", "cep_empresa");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", regimeTrib);
    query.bindValue(":key", "regime_trib");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", tpAmb);
    query.bindValue(":key", "tp_amb");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", idCsc);
    query.bindValue(":key", "id_csc");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", csc);
    query.bindValue(":key", "csc");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", pastaSchema);
    query.bindValue(":key", "caminho_schema");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", pastaCertificadoAc);
    query.bindValue(":key", "caminho_certac");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", certificado);
    query.bindValue(":key", "caminho_certificado");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", senhaCertificado);
    query.bindValue(":key", "senha_certificado");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", cUf);
    query.bindValue(":key", "cuf");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", cMun);
    query.bindValue(":key", "cmun");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", iEstad);
    query.bindValue(":key", "iest");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", cnpjRT);
    query.bindValue(":key", "cnpj_rt");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", nomeRT);
    query.bindValue(":key", "nome_rt");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", emailRt);
    query.bindValue(":key", "email_rt");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", foneRt);
    query.bindValue(":key", "fone_rt");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", idCSRT);
    query.bindValue(":key", "id_csrt");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", hasCsrt);
    query.bindValue(":key", "hash_csrt");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", emitNf);
    query.bindValue(":key", "emit_nf");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", nnfHomolog);
    query.bindValue(":key", "nnf_homolog");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", nnfProd);
    query.bindValue(":key", "nnf_prod");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", csosnPadrao);
    query.bindValue(":key", "csosn_padrao");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", ncmPadrao);
    query.bindValue(":key", "ncm_padrao");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", cestPadrao);
    query.bindValue(":key", "cest_padrao");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", pisPadrao);
    query.bindValue(":key", "pis_padrao");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", nnfProdNfe);
    query.bindValue(":key", "nnf_prod_nfe");
    query.exec();
    query.prepare("UPDATE config set value = :value WHERE key = :key");
    query.bindValue(":value", nnfHomologNfe);
    query.bindValue(":key", "nnf_homolog_nfe");
    query.exec();


    db.close();

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

    // Obter o diretório de dados do aplicativo (mesmo local do estoque.db)
    QString dataDir;

#if defined(Q_OS_LINUX)
    dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_WIN)
    dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif

    // Criar subdiretório para imagens
    QString pastaImagens = dataDir + "/imagens";
    QDir dir;
    if (!dir.exists(pastaImagens)) {
        dir.mkpath(pastaImagens);
    }

    // Preparar novo caminho com nome único
    QFileInfo info(caminhoOriginal);
    QString novoCaminho = pastaImagens + "/" + info.fileName();

    // Evitar sobreposição de arquivos
    int contador = 1;
    while (QFile::exists(novoCaminho)) {
        novoCaminho = pastaImagens + "/" + info.baseName() + "_" + QString::number(contador++) + "." + info.completeSuffix();
    }

    // Copiar a imagem
    if (QFile::copy(caminhoOriginal, novoCaminho)) {
        // Armazenar apenas o nome do arquivo ou caminho relativo
        //ui->Ledt_LogoEmpresa->setText(QFileInfo(novoCaminho).fileName());

        // Alternativa: armazenar caminho relativo à pasta de imagens
         ui->Ledt_LogoEmpresa->setText("imagens/" + QFileInfo(novoCaminho).fileName());
    } else {
        QMessageBox::warning(this, "Erro", "Não foi possível copiar a imagem.");
    }
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


void Configuracao::on_Btn_CertficadoAcPath_clicked()
{
    QString pastaSelecionada = QFileDialog::getExistingDirectory(
        this,
        tr("Selecionar a Pasta dos Certificaods AC"),
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (pastaSelecionada.isEmpty())
        return;

    ui->Lbl_CertificadoAcPath->setText(pastaSelecionada);
}


QMap<QString, QString> Configuracao::get_All_Fiscal_Values() {

    QSqlDatabase db2 = QSqlDatabase::database();
    if(!db2.open()){
        qDebug() << "erro bancodedados";
    }
    QMap<QString, QString> result;

    // Lista fixa de chaves que queremos buscar
    QStringList keys = {
        "regime_trib",
        "tp_amb",
        "id_csc",
        "csc",
        "caminho_schema",
        "caminho_certac",
        "caminho_certificado",
        "senha_certificado",
        "cuf",
        "cmun",
        "iest",
        "cnpj_rt",
        "nome_rt",
        "email_rt",
        "fone_rt",
        "id_csrt",
        "hash_csrt",
        "emit_nf",
        "nnf_homolog",
        "nnf_prod",
        "nnf_homolog_nfe",
        "nnf_prod_nfe"
    };

    // Montar a query com placeholders
    QStringList placeholders;
    for (int i = 0; i < keys.size(); ++i) {
        placeholders << "?";
    }

    QString queryStr = QString("SELECT key, value FROM config WHERE key IN (%1)")
                           .arg(placeholders.join(", "));

    QSqlQuery query;
    query.prepare(queryStr);

    // Vincular os valores
    for (const QString &key : keys) {
        query.addBindValue(key);
    }

    if (!query.exec()) {
        qWarning() << "Erro ao executar a query de configuração:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        QString key = query.value("key").toString();
        QString value = query.value("value").toString();
        result.insert(key, value);
    }

    return result;
}

QMap<QString, QString> Configuracao::get_All_Empresa_Values() {

    QSqlDatabase db2 = QSqlDatabase::database();
    if(!db2.open()){
        qDebug() << "erro bancodedados";
    }
    QMap<QString, QString> result;

    // Lista fixa de chaves que queremos buscar
    QStringList keys = {
        "nome_empresa",
        "endereco_empresa",
        "telefone_empresa",
        "cnpj_empresa",
        "email_empresa",
        "cidade_empresa",
        "estado_empresa",
        "caminho_logo_empresa",
        "nfant_empresa",
        "numero_empresa",
        "bairro_empresa",
        "cep_empresa"
    };

    // Montar a query com placeholders
    QStringList placeholders;
    for (int i = 0; i < keys.size(); ++i) {
        placeholders << "?";
    }

    QString queryStr = QString("SELECT key, value FROM config WHERE key IN (%1)")
                           .arg(placeholders.join(", "));

    QSqlQuery query;
    query.prepare(queryStr);

    // Vincular os valores
    for (const QString &key : keys) {
        query.addBindValue(key);
    }

    if (!query.exec()) {
        qWarning() << "Erro ao executar a query de configuração:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        QString key = query.value("key").toString();
        QString value = query.value("value").toString();
        result.insert(key, value);
    }

    return result;
}

QMap<QString, QString> Configuracao::get_All_Financeiro_Values(){

    QSqlDatabase db2 = QSqlDatabase::database();
    if(!db2.open()){
        qDebug() << "erro bd get all financeiro values config";
    }
    QMap<QString, QString> result;

    // Lista fixa de chaves que queremos buscar
    QStringList keys = {
        "porcent_lucro",
        "taxa_debito",
        "taxa_credito"
    };

    // Montar a query com placeholders
    QStringList placeholders;
    for (int i = 0; i < keys.size(); ++i) {
        placeholders << "?";
    }

    QString queryStr = QString("SELECT key, value FROM config WHERE key IN (%1)")
                           .arg(placeholders.join(", "));

    QSqlQuery query;
    query.prepare(queryStr);

    // Vincular os valores
    for (const QString &key : keys) {
        query.addBindValue(key);
    }

    if (!query.exec()) {
        qWarning() << "Erro ao executar a query de configuração:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        QString key = query.value("key").toString();
        QString value = query.value("value").toString();
        result.insert(key, value);
    }

    return result;
}

QMap<QString, QString> Configuracao::get_All_Produto_Values(){

    QSqlDatabase db2 = QSqlDatabase::database();
    if(!db2.open()){
        qDebug() << "erro bd get all financeiro values config";
    }
    QMap<QString, QString> result;

    // Lista fixa de chaves que queremos buscar
    QStringList keys = {
        "ncm_padrao",
        "csosn_padrao",
        "cest_padrao",
        "pis_padrao"
    };

    // Montar a query com placeholders
    QStringList placeholders;
    for (int i = 0; i < keys.size(); ++i) {
        placeholders << "?";
    }

    QString queryStr = QString("SELECT key, value FROM config WHERE key IN (%1)")
                           .arg(placeholders.join(", "));

    QSqlQuery query;
    query.prepare(queryStr);

    // Vincular os valores
    for (const QString &key : keys) {
        query.addBindValue(key);
    }

    if (!query.exec()) {
        qWarning() << "Erro ao executar a query de configuração:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        QString key = query.value("key").toString();
        QString value = query.value("value").toString();
        result.insert(key, value);
    }

    return result;
}


void Configuracao::on_label_35_linkActivated(const QString &link)
{
    if (link == "help") {
        HelpPage *help = new HelpPage(this);
        help->show();
    }
}

