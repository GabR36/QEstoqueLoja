#include "inserircliente.h"
#include "ui_inserircliente.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QRegularExpression>
#include <QSqlError>
#include "util/ACBrConsultaCNPJ.h"
#include <QDebug>
#include <QStandardPaths>
#include <QRegularExpression>
#include "util/consultacnpjmanager.h"
#include "util/buscarcodigomunicipio.h"

InserirCliente::InserirCliente(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InserirCliente)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool);

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
    QString nome = ui->Ledit_Nome->text().trimmed();
    QString email = ui->Ledit_Email->text().trimmed();
    QString telefone = ui->Ledit_Telefone->text().trimmed();
    QString endereco = ui->Ledit_Endereco->text().trimmed();
    QString cpf = ui->Ledit_Cpf->text().trimmed();
    QString dataNasc = ui->Ledit_DataNascimento->text().trimmed();
    QString numero = ui->Ledit_Numero->text().trimmed();
    QString bairro = ui->Ledit_Bairro->text().trimmed();
    QString xMun = ui->Ledit_Municipio->text().trimmed();
    QString cMun = ui->Ledit_CMun->text().trimmed();
    QString uf = ui->Ledit_UF->text().trimmed();
    QString cep = ui->Ledit_CEP->text().trimmed();
    QString ie = ui->Ledit_IE->text().trimmed();
    int indIeDest = ui->CBox_IndIEDest->currentIndex();
    bool ehPf;
    if(ui->RadioB_Pfisica->isChecked()){
        ehPf = true;
    }else{
        ehPf = false;
    }
    // Verificação do Nome (Único campo obrigatório)
    if (nome.isEmpty()) {
        QMessageBox::warning(this, "Erro", "Por favor, insira um nome.");
        return;
    }

    // Validação do CPF (Apenas se o campo não estiver vazio)
    if (!cpf.isEmpty() && (cpf.length() != 11 || !cpf.toLongLong()) && ui->RadioB_Pfisica->isChecked()) {
        QMessageBox::warning(this, "Erro", "CPF inválido! Insira um CPF com 11 números.");
        return;
    }
    // Validação do CPF (Apenas se o campo não estiver vazio)
    if (!cpf.isEmpty() && (cpf.length() != 14 || !cpf.toLongLong()) && ui->RadioB_Pjuridica->isChecked()) {
        QMessageBox::warning(this, "Erro", "cnpj inválido! Insira um cnpj com 14 números.");
        return;
    }

    // Validação do Email (Apenas se o campo não estiver vazio)
    if (!email.isEmpty()) {
        QRegularExpression emailRegex(R"((\w+)(\.\w+)*@(\w+)((\.\w+)+))");
        if (!emailRegex.match(email).hasMatch()) {
            QMessageBox::warning(this, "Erro", "Email inválido! Insira um email válido.");
            return;
        }
    }

    // Validação do Telefone (Apenas se o campo não estiver vazio)
    if (!telefone.isEmpty() && (telefone.length() < 10 || !telefone.toLongLong())) {
        QMessageBox::warning(this, "Erro", "Telefone inválido! Insira um número de telefone válido.");
        return;
    }

    if((ui->CBox_IndIEDest->currentIndex() == 1 || ui->CBox_IndIEDest->currentIndex() == 2)
            && ui->Ledit_IE->text().isEmpty()){
        QMessageBox::warning(this, "Erro", "Inscrição Estadual precisa ser preenchida"
                                           " quando IndIeDest = contribuinte");
        return;
    }

    // Inserir no banco de dados
    if (!db.open()) {
        QMessageBox::warning(this, "Erro", "Erro ao conectar ao banco de dados.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO clientes (nome, email, telefone, endereco, cpf,"
                  " data_nascimento, eh_pf, numero_end, bairro, xMun, cMun, uf,"
                  "cep, indIEDest, ie) "
                  "VALUES (:nome, :email, :telefone, :endereco, :cpf, :dataNasc, :ehPf, :num, "
                  ":bairro, :xmun, :cmun, :uf, :cep, :indiedest, :ie)");
    query.bindValue(":nome", nome);
    query.bindValue(":email", email.isEmpty() ? QVariant(QVariant::String) : email);
    query.bindValue(":telefone", telefone.isEmpty() ? QVariant(QVariant::String) : telefone);
    query.bindValue(":endereco", endereco.isEmpty() ? QVariant(QVariant::String) : endereco);
    query.bindValue(":cpf", cpf.isEmpty() ? QVariant(QVariant::String) : cpf);
    query.bindValue(":dataNasc", dataNasc.isEmpty() ? QVariant(QVariant::String) : dataNasc);
    query.bindValue(":ehPf", ehPf);
    query.bindValue(":num", numero);
    query.bindValue(":bairro", bairro);
    query.bindValue(":xmun", xMun);
    query.bindValue(":cmun", cMun);
    query.bindValue(":uf", uf);
    query.bindValue(":cep", cep);
    query.bindValue(":indiedest", indIeDest);
    query.bindValue(":ie", ie);


    if (!query.exec()) {
        qDebug() << "Erro ao inserir cliente: " << query.lastError().text();
        QMessageBox::warning(this, "Erro", "Erro ao inserir cliente no banco de dados.");
    } else {
        QMessageBox::information(this, "Sucesso", "Cliente inserido com sucesso!");
        emit clienteInserido();
        this->close();
    }

    db.close();
}

void InserirCliente::on_Btn_BuscaDados_clicked()
{
    // auto manager = ConsultaCnpjManager::instance();
    auto cnpj = ConsultaCnpjManager::instance()->cnpj();

    QString cnpjSt = ui->Ledit_Cpf->text();

    QString retorno;
    try {
        std::string resultado = cnpj->Consultar(cnpjSt.toStdString());
        retorno = QString::fromStdString(resultado);
        qDebug() << "Consulta retornou:" << retorno.left(200);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Erro", QString("Falha na consulta: %1").arg(e.what()));
        // manager->resetLib();
        return;

    }

    QStringList linhas = retorno.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    QString nomeRet, enderecoRet, numeroRet, bairroRet, municipioRet, data_nascRet,
        cepRet, ufRet, ieRet;

    for (const QString &linha : linhas) {
        if (linha.startsWith("RazaoSocial="))
            nomeRet = linha.section('=', 1).trimmed();
        else if (linha.startsWith("Endereco="))
            enderecoRet = linha.section('=', 1).trimmed();
        else if (linha.startsWith("Numero="))
            numeroRet = linha.section('=', 1).trimmed();
        else if (linha.startsWith("Bairro="))
            bairroRet = linha.section('=', 1).trimmed();
        else if (linha.startsWith("Cidade="))
            municipioRet = linha.section('=', 1).trimmed();
        else if (linha.startsWith("Abertura="))
            data_nascRet = linha.section('=', 1).trimmed();
        else if (linha.startsWith("CEP="))
            cepRet = linha.section('=', 1).trimmed();
        else if (linha.startsWith("UF="))
            ufRet = linha.section('=', 1).trimmed();
        else if (linha.startsWith("InscricaoEstadual="))
            ieRet = linha.section('=', 1).trimmed();
    }
    // QMessageBox::information(this, "teste", retorno);

    ui->Ledit_Nome->setText(nomeRet);
    ui->Ledit_Endereco->setText(enderecoRet);
    ui->Ledit_Numero->setText(numeroRet);
    ui->Ledit_Bairro->setText(bairroRet);
    ui->Ledit_Municipio->setText(municipioRet);
    ui->Ledit_DataNascimento->setText(data_nascRet);
    ui->Ledit_CEP->setText(cepRet);
    ui->Ledit_UF->setText(ufRet);
    ui->Ledit_IE->setText(ieRet);

    //se tem IE, indicador ie dest = contribuinte icms
    if(!ieRet.isEmpty()){
        ui->CBox_IndIEDest->setCurrentIndex(1);
    }else{
        ui->CBox_IndIEDest->setCurrentIndex(0);
    }

    QString cmun = BuscarCodigoMunicipio::buscarCodigoMunicipio(municipioRet);
    ui->Ledit_CMun->setText(cmun);
}

