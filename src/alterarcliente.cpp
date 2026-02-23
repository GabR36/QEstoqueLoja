#include "alterarcliente.h"
#include "ui_alterarcliente.h"
#include <QSqlQuery>
#include <QMessageBox>
#include "util/consultacnpjmanager.h"
#include "util/buscarcodigomunicipio.h"
#include "services/consultacnpj_service.h"

AlterarCliente::AlterarCliente(QWidget *parent, QString id)
    : QWidget(parent)
    , ui(new Ui::AlterarCliente)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool);

    qDebug() << "id alterar cliente: " + id;

    //aplicar validators
    QIntValidator *intValidator = new QIntValidator(1, 999999, this);
    ui->Ledit_Numero->setValidator(intValidator);
    //ui->Ledit_Telefone->setValidator(intValidator);
    QRegularExpression rx("[A-Za-zÀ-ÿ\\s]{1,50}");
    QRegularExpressionValidator *textValidator = new QRegularExpressionValidator(rx, this);
    ui->Ledit_Bairro->setValidator(textValidator);
    ui->Ledit_Municipio->setValidator(textValidator);

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
        new QRegularExpressionValidator(QRegularExpression("^[A-Za-zÀ-ÿ ]{1,30}$"), this);    ui->Ledit_Bairro->setValidator(sohLetraValidator);
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



    ui->Ledit_DataNascimento->setInputMask("99/99/9999");

    //max lenght
    ui->Ledit_Telefone->setMaxLength(12);
    ui->Ledit_Cpf->setMaxLength(15);
    ui->Ledit_Nome->setMaxLength(70);
    ui->Ledit_Endereco->setMaxLength(60);
    ui->Ledit_Email->setMaxLength(60);

    ClienteDTO cli = cliServ.getClienteByID(id.toLongLong());

    // colocar os valores nos lineedits
    ui->Ledit_Cpf->setText(cli.cpf);
    ui->Ledit_DataNascimento->setText(cli.dataNasc);
    ui->Ledit_Email->setText(cli.email);
    ui->Ledit_Endereco->setText(cli.endereco);
    ui->Ledit_Nome->setText(cli.nome);
    ui->Ledit_Telefone->setText(cli.telefone);
    ui->Ledit_Numero->setText(QString::number(cli.numeroEnd));
    ui->Ledit_Bairro->setText(cli.bairro);
    ui->Ledit_Municipio->setText(cli.xMun);
    ui->Ledit_CMun->setText(cli.cMun);
    ui->Ledit_UF->setText(cli.uf);
    ui->Ledit_CEP->setText(cli.cep);
    ui->CBox_IndIEDest->setCurrentIndex(cli.indIeDest);
    ui->Ledit_IE->setText(cli.ie);
    if(cli.ehPf){
        ui->RadioB_Pfisica->setChecked(true);
    }else{
        ui->RadioB_Pjuridica->setChecked(true);
    }


    db.close();

    this->id = id;
}

AlterarCliente::~AlterarCliente()
{
    delete ui;
}

void AlterarCliente::on_Btn_Cancelar_clicked()
{
    this->close();
}


void AlterarCliente::on_Btn_Ok_clicked()
{
    // pegar os valores dos lineedits para colocar no banco de dados
    ClienteDTO cli;

    cli.nome = ui->Ledit_Nome->text().trimmed();
    cli.email = ui->Ledit_Email->text().trimmed();
    cli.telefone = ui->Ledit_Telefone->text().trimmed();
    cli.endereco = ui->Ledit_Endereco->text().trimmed();
    cli.cpf = ui->Ledit_Cpf->text().trimmed();
    cli.dataNasc = ui->Ledit_DataNascimento->text().trimmed();
    if(ui->RadioB_Pfisica->isChecked()){
        cli.ehPf = true;
    }else{
        cli.ehPf = false;
    }
    cli.numeroEnd = ui->Ledit_Numero->text().trimmed().toLongLong();
    cli.bairro = ui->Ledit_Bairro->text().trimmed();
    cli.xMun = ui->Ledit_Municipio->text().trimmed();
    cli.cMun = ui->Ledit_CMun->text().trimmed();
    cli.uf = ui->Ledit_UF->text().trimmed();
    cli.cep = ui->Ledit_CEP->text().trimmed();
    cli.ie = ui->Ledit_IE->text().trimmed();
    cli.indIeDest = ui->CBox_IndIEDest->currentIndex();

    auto result = cliServ.updateCliente(id.toLongLong(), cli);
    if(!result.ok){
        QMessageBox::warning(this, "Erro", result.msg);
    }else{
        QMessageBox::information(this, "Sucesso", "Cliente alterado com sucesso!");
        emit clienteAtualizado();
        this->close();
    }

}

void AlterarCliente::on_Btn_BuscaDados_clicked()
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
