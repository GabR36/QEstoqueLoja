#include "config.h"
#include "ui_config.h"

Config::Config(QWidget *parent)
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
    ui->Ledt_LucroEmpresa->setText(configValues.value("porcent_lucro", ""));
    ui->Ledt_NomeEmpresa->setText(configValues.value("nome_empresa", ""));
    ui->Ledt_TelEmpresa->setText(configValues.value("telefone_empresa", ""));

    db.close();
}

Config::~Config()
{
    delete ui;
}


void Config::on_Btn_Aplicar_clicked()
{
    QString nomeEmpresa, enderecoEmpresa, emailEmpresa, cnpjEmpresa, telEmpresa, lucro;
    nomeEmpresa = ui->Ledt_NomeEmpresa->text();
    enderecoEmpresa = ui->Ledt_EnderecoEmpresa->text();
    emailEmpresa = ui->Ledt_EnderecoEmpresa->text();
    cnpjEmpresa = ui->Ledt_CnpjEmpresa->text();
    telEmpresa = ui->Ledt_TelEmpresa->text();
    lucro = ui->Ledt_LucroEmpresa->text();

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

    db.close();

    this->close();
}


void Config::on_Btn_Cancelar_clicked()
{
    this->close();
}

