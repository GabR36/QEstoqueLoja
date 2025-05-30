#include "configuracao.h"
#include "ui_configuracao.h"
#include <QFileDialog>
#include <QMessageBox>

Configuracao::Configuracao(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Config)
{
    ui->setupUi(this);

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
    // converter notacao americana em ptbr
    ui->Ledt_LucroEmpresa->setText(portugues.toString(configValues.value("porcent_lucro", "").toFloat()));
    ui->Ledt_debito->setText(portugues.toString(configValues.value("taxa_debito", "").toFloat()));
    ui->Ledt_credito->setText(portugues.toString(configValues.value("taxa_credito", "").toFloat()));

    db.close();

    // validador
    QDoubleValidator *validador = new QDoubleValidator();
    ui->Ledt_LucroEmpresa->setValidator(validador);
    ui->Ledt_credito->setValidator(validador);
    ui->Ledt_debito->setValidator(validador);
}

Configuracao::~Configuracao()
{
    delete ui;
}


void Configuracao::on_Btn_Aplicar_clicked()
{
    QString nomeEmpresa, enderecoEmpresa, emailEmpresa, cnpjEmpresa, telEmpresa, lucro,
        debito, credito, cidadeEmpresa, estadoEmpresa, logoEmpresa;
    // converter para notacao americana para armazenar no banco de dados
    nomeEmpresa = ui->Ledt_NomeEmpresa->text();
    enderecoEmpresa = ui->Ledt_EnderecoEmpresa->text();
    emailEmpresa = ui->Ledt_EmailEmpresa->text();
    cnpjEmpresa = ui->Ledt_CnpjEmpresa->text();
    telEmpresa = ui->Ledt_TelEmpresa->text();
    cidadeEmpresa = ui->Ledt_CidadeEmpresa->text();
    estadoEmpresa = ui->Ledt_EstadoEmpresa->text();
    logoEmpresa = ui->Ledt_LogoEmpresa->text();
    lucro = QString::number(portugues.toFloat(ui->Ledt_LucroEmpresa->text()));
    debito = QString::number(portugues.toFloat(ui->Ledt_debito->text()));
    credito = QString::number(portugues.toFloat(ui->Ledt_credito->text()));

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

    // Criar diretório de destino se não existir
    QString pastaDestino = QCoreApplication::applicationDirPath() + "/imagens";
    QDir dir;
    if (!dir.exists(pastaDestino)) {
        dir.mkpath(pastaDestino);
    }

    // Copiar arquivo para o diretório do projeto com um nome único
    QFileInfo info(caminhoOriginal);
    QString nomeArquivo = info.fileName();
    QString novoCaminho = pastaDestino + "/" + nomeArquivo;

    // Verifica se já existe um arquivo com esse nome e adiciona sufixo se necessário
    int contador = 1;
    while (QFile::exists(novoCaminho)) {
        novoCaminho = pastaDestino + "/" + info.baseName() + "_" + QString::number(contador++) + "." + info.completeSuffix();
    }

    // Copiar o arquivo
    if (QFile::copy(caminhoOriginal, novoCaminho)) {
        // Atualiza o line edit com o novo caminho
        QString caminhoRelativo = "imagens/" + QFileInfo(novoCaminho).fileName();
        ui->Ledt_LogoEmpresa->setText(caminhoRelativo);
    } else {
        QMessageBox::warning(this, "Erro", "Não foi possível copiar a imagem.");
    }

}

