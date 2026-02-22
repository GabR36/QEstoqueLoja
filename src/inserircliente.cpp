#include "inserircliente.h"
#include "ui_inserircliente.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QRegularExpression>
#include "util/consultacnpjmanager.h"
#include "util/buscarcodigomunicipio.h"
#include "infra/databaseconnection_service.h"
#include "services/consultacnpj_service.h"

InserirCliente::InserirCliente(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InserirCliente)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool);
    db = DatabaseConnection_service::db();

    //aplicar validators
    QIntValidator *intValidator = new QIntValidator(1, 999999, this);
    ui->Ledit_Numero->setValidator(intValidator);
    //ui->Ledit_Telefone->setValidator(intValidator);
    QRegularExpression rx("[A-Za-zÀ-ÿ\\s]{1,50}");
    QRegularExpressionValidator *textValidator = new QRegularExpressionValidator(rx, this);
    ui->Ledit_Bairro->setValidator(textValidator);
    ui->Ledit_Municipio->setValidator(textValidator);
    QRegularExpression sohNumeroRegex("^[0-9]*$");
    QRegularExpressionValidator *soNumeroValidator = new QRegularExpressionValidator(sohNumeroRegex, this);

    QRegularExpressionValidator *ibgeValidator =
        new QRegularExpressionValidator(QRegularExpression("\\d{7}"), this);
    ui->Ledit_CMun->setValidator(ibgeValidator);

    QRegularExpressionValidator *ufValidator =
        new QRegularExpressionValidator(QRegularExpression("[A-Z]{2}"), this);
    ui->Ledit_UF->setValidator(ufValidator);

    QRegularExpressionValidator *cepValidator =
        new QRegularExpressionValidator(QRegularExpression("\\d{8}"), this);
    ui->Ledit_CEP->setValidator(cepValidator);

    QRegularExpressionValidator *ieValidator =
        new QRegularExpressionValidator(QRegularExpression("\\d{0,14}"), this);
    ui->Ledit_IE->setValidator(ieValidator);

    QRegularExpressionValidator *sohLetraValidator =
        new QRegularExpressionValidator(QRegularExpression("^[A-Za-zÀ-ÿ ]{1,30}$"), this);
    ui->Ledit_Bairro->setValidator(sohLetraValidator);
    ui->Ledit_Municipio->setValidator(sohLetraValidator);
    QRegularExpressionValidator *emailValidator =
        new QRegularExpressionValidator(
            QRegularExpression("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$"),this);
    ui->Ledit_Email->setValidator(emailValidator);
    QRegularExpressionValidator *cnpjValidator =
        new QRegularExpressionValidator(
            QRegularExpression("^\\d{14}$"),
            this
            );
    ui->Ledit_Cpf->setValidator(cnpjValidator);
    ui->Ledit_Telefone->setValidator(soNumeroValidator);



    ui->Ledit_DataNascimento->setInputMask("99/99/9999");

    //max lenght
    ui->Ledit_Telefone->setMaxLength(12);
    ui->Ledit_Cpf->setMaxLength(15);
    ui->Ledit_Nome->setMaxLength(70);
    ui->Ledit_Endereco->setMaxLength(60);
    ui->Ledit_Email->setMaxLength(60);


}

InserirCliente::~InserirCliente()
{
    delete ui;
}

void InserirCliente::on_Btn_Cancelar_clicked()
{
    this->close();
}

void InserirCliente::on_Btn_Inserir_clicked()
{
    // Pegar os valores dos campos de entrada

    ClienteDTO newCli;

    newCli.nome = ui->Ledit_Nome->text().trimmed();
    newCli.email = ui->Ledit_Email->text().trimmed();
    newCli.telefone = ui->Ledit_Telefone->text().trimmed();
    newCli.endereco = ui->Ledit_Endereco->text().trimmed();
    newCli.cpf = ui->Ledit_Cpf->text().trimmed();
    newCli.dataNasc = ui->Ledit_DataNascimento->text().trimmed();
    newCli.numeroEnd = ui->Ledit_Numero->text().trimmed().toLongLong();
    newCli.bairro = ui->Ledit_Bairro->text().trimmed();
    newCli.xMun = ui->Ledit_Municipio->text().trimmed();
    newCli.cMun = ui->Ledit_CMun->text().trimmed();
    newCli.uf = ui->Ledit_UF->text().trimmed();
    newCli.cep = ui->Ledit_CEP->text().trimmed();
    newCli.ie = ui->Ledit_IE->text().trimmed();
    newCli.indIeDest = ui->CBox_IndIEDest->currentIndex();

    if(ui->RadioB_Pfisica->isChecked()){
        newCli.ehPf = true;
    }else{
        newCli.ehPf = false;
    }

    auto result = cliServ.inserirCliente(newCli);

    if(!result.ok){
        QMessageBox::warning(this, "Erro", result.msg);
    }else{
        this->close();
        emit clienteInserido();
    }
}

void InserirCliente::on_Btn_BuscaDados_clicked()
{
    ConsultaCnpj_service cnpjServ;
    QString cnpj = ui->Ledit_Cpf->text();
    ClienteDTO clienteCNPJ = cnpjServ.getInfo(cnpj);

    ui->Ledit_Nome->setText(clienteCNPJ.nome);
    ui->Ledit_Endereco->setText(clienteCNPJ.endereco);
    ui->Ledit_Numero->setText(QString::number(clienteCNPJ.numeroEnd));
    ui->Ledit_Bairro->setText(clienteCNPJ.bairro);
    ui->Ledit_Municipio->setText(clienteCNPJ.xMun);
    ui->Ledit_DataNascimento->setText(clienteCNPJ.dataNasc);
    ui->Ledit_CEP->setText(clienteCNPJ.cep);
    ui->Ledit_UF->setText(clienteCNPJ.uf);
    ui->Ledit_IE->setText(clienteCNPJ.ie);

    QString cmun = BuscarCodigoMunicipio::buscarCodigoMunicipio(clienteCNPJ.xMun);
    ui->Ledit_CMun->setText(cmun);
}


