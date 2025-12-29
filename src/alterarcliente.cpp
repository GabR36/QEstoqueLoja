#include "alterarcliente.h"
#include "ui_alterarcliente.h"
#include <QSqlQuery>
#include <QMessageBox>
#include "util/consultacnpjmanager.h"
#include "util/buscarcodigomunicipio.h"

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

    // obter as informações do cliente para colocar nos linedits
    if(!db.open()){
        qDebug() << "db não abriu, janela alterar cliente";
    }
    QSqlQuery query;
    query.prepare("SELECT nome, email, telefone, endereco, cpf, data_nascimento, "
                  "eh_pf, numero_end, bairro, xMun, cMun, uf, cep, indIEDest, ie "
                  "FROM clientes "
                  "WHERE id = :valor1");
    query.bindValue(":valor1", id);
    if(!query.exec()){
        qDebug() << "Query erro, alterar cliente.";
    }
    QString nome, email, telefone, endereco, cpf, dataNasc, numero, bairro,
        xmun, cmun, uf, cep, ie;
    bool ehPf = false;
    int indiedest;
    query.next();
    nome = query.value(0).toString();
    email = query.value(1).toString();
    telefone = query.value(2).toString();
    endereco = query.value(3).toString();
    cpf = query.value(4).toString();
    dataNasc = query.value(5).toString();
    ehPf = query.value(6).toBool();
    numero = query.value(7).toString();
    bairro = query.value(8).toString();
    xmun = query.value(9).toString();
    cmun = query.value(10).toString();
    uf = query.value(11).toString();
    cep = query.value(12).toString();
    indiedest = query.value(13).toInt();
    ie = query.value(14).toString();

    qDebug() << "valores alterar cliente: " + nome + email + telefone + endereco + cpf +
                    dataNasc << ehPf;

    // colocar os valores nos lineedits
    ui->Ledit_Cpf->setText(cpf);
    ui->Ledit_DataNascimento->setText(dataNasc);
    ui->Ledit_Email->setText(email);
    ui->Ledit_Endereco->setText(endereco);
    ui->Ledit_Nome->setText(nome);
    ui->Ledit_Telefone->setText(telefone);
    ui->Ledit_Numero->setText(numero);
    ui->Ledit_Bairro->setText(bairro);
    ui->Ledit_Municipio->setText(xmun);
    ui->Ledit_CMun->setText(cmun);
    ui->Ledit_UF->setText(uf);
    ui->Ledit_CEP->setText(cep);
    ui->CBox_IndIEDest->setCurrentIndex(indiedest);
    ui->Ledit_IE->setText(ie);
    if(ehPf){
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
    QString nome, email, telefone, endereco, cpf, dataNasc, numero, bairro,
        xmun, cmun, uf, cep, ie;
    int iedest;
    bool ehPf;
    nome = ui->Ledit_Nome->text().trimmed();
    email = ui->Ledit_Email->text().trimmed();
    telefone = ui->Ledit_Telefone->text().trimmed();
    endereco = ui->Ledit_Endereco->text().trimmed();
    cpf = ui->Ledit_Cpf->text().trimmed();
    dataNasc = ui->Ledit_DataNascimento->text().trimmed();
    if(ui->RadioB_Pfisica->isChecked()){
        ehPf = true;
    }else{
        ehPf = false;
    }
    numero = ui->Ledit_Numero->text().trimmed();
    bairro = ui->Ledit_Bairro->text().trimmed();
    xmun = ui->Ledit_Municipio->text().trimmed();
    cmun = ui->Ledit_CMun->text().trimmed();
    uf = ui->Ledit_UF->text().trimmed();
    cep = ui->Ledit_CEP->text().trimmed();
    ie = ui->Ledit_IE->text().trimmed();
    iedest = ui->CBox_IndIEDest->currentIndex();
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



    // impedir nome de ser vazio
    if (nome == ""){
        QMessageBox::warning(this, "Erro", "Por favor, insira um nome.");
    }
    else{
        // alterar os valores no banco
        db.open();

        QSqlQuery query;
        query.prepare("UPDATE clientes SET nome = :valor1, email = :valor2, "
                      "telefone = :valor3, endereco = :valor4, cpf = :valor5, "
                      "data_nascimento = :valor6, eh_pf = :valor7, numero_end = :numero, "
                      "bairro = :bairro, xMun = :xmun, cMun = :cmun, uf = :uf, cep = :cep, "
                      "indIEDest = :indiedest, ie = :ie WHERE id = :valor8");
        query.bindValue(":valor1", nome);
        query.bindValue(":valor2", email);
        query.bindValue(":valor3", telefone);
        query.bindValue(":valor4", endereco);
        query.bindValue(":valor5", cpf);
        query.bindValue(":valor6", dataNasc);
        query.bindValue(":valor7", ehPf);
        query.bindValue(":valor8", id);
        query.bindValue(":numero", numero);
        query.bindValue(":bairro", bairro);
        query.bindValue(":xmun", xmun);
        query.bindValue(":cmun", cmun);
        query.bindValue(":uf", uf);
        query.bindValue(":cep", cep);
        query.bindValue(":indiedest", iedest);
        query.bindValue(":ie", ie);
        if(!query.exec()){
            qDebug() << "erro query, update clientes";
        }

        db.close();
        QMessageBox::information(this, "Sucesso", "Cliente alterado com sucesso!");
        emit clienteAtualizado();
        this->close();
    }
}


void AlterarCliente::on_Btn_BuscaDados_clicked()
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

