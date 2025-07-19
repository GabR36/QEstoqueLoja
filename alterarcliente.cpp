#include "alterarcliente.h"
#include "ui_alterarcliente.h"
#include <QSqlQuery>
#include <QMessageBox>

AlterarCliente::AlterarCliente(QWidget *parent, QString id)
    : QWidget(parent)
    , ui(new Ui::AlterarCliente)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool);

    qDebug() << "id alterar cliente: " + id;
    //validadores
    QIntValidator *intValidator = new QIntValidator(1, 999999, this);
    ui->Ledit_Numero->setValidator(intValidator);
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
        emit clienteAtualizado();

        this->close();
    }
}

