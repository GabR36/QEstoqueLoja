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

    // obter as informações do cliente para colocar nos linedits
    if(!db.open()){
        qDebug() << "db não abriu, janela alterar cliente";
    }
    QSqlQuery query;
    query.prepare("SELECT nome, email, telefone, endereco, cpf, data_nascimento, eh_pf FROM clientes "
                  "WHERE id = :valor1");
    query.bindValue(":valor1", id);
    if(!query.exec()){
        qDebug() << "Query erro, alterar cliente.";
    }
    QString nome, email, telefone, endereco, cpf, dataNasc;
    bool ehPf = false;
    query.next();
    nome = query.value(0).toString();
    email = query.value(1).toString();
    telefone = query.value(2).toString();
    endereco = query.value(3).toString();
    cpf = query.value(4).toString();
    dataNasc = query.value(5).toString();
    ehPf = query.value(6).toBool();

    qDebug() << "valores alterar cliente: " + nome + email + telefone + endereco + cpf + dataNasc;

    // colocar os valores nos lineedits
    ui->Ledit_Cpf->setText(cpf);
    ui->Ledit_DataNascimento->setText(dataNasc);
    ui->Ledit_Email->setText(email);
    ui->Ledit_Endereco->setText(endereco);
    ui->Ledit_Nome->setText(nome);
    ui->Ledit_Telefone->setText(telefone);
    if(!ehPf){
        ui->RadioB_Pjuridica->setChecked(true);
    }else{
        ui->RadioB_Pfisica->setChecked(true);
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
    QString nome, email, telefone, endereco, cpf, dataNasc;
    nome = ui->Ledit_Nome->text();
    email = ui->Ledit_Email->text();
    telefone = ui->Ledit_Telefone->text();
    endereco = ui->Ledit_Cpf->text();
    cpf = ui->Ledit_Cpf->text();
    dataNasc = ui->Ledit_DataNascimento->text();
    bool ehPf;
    if(ui->RadioB_Pfisica->isChecked()){
        ehPf = true;
    }else{
        ehPf = false;
    }

    // impedir nome de ser vazio
    if (nome == ""){
        QMessageBox::warning(this, "Erro", "Por favor, insira um nome.");
    }
    else{
        // alterar os valores no banco
        db.open();

        QSqlQuery query;
        query.prepare("UPDATE clientes SET nome = :valor1, email = :valor2, telefone = :valor3, "
                      "endereco = :valor4, cpf = :valor5, data_nascimento = :valor6, eh_pf = :valor7 WHERE id = :valor8");
        query.bindValue(":valor1", nome);
        query.bindValue(":valor2", email);
        query.bindValue(":valor3", telefone);
        query.bindValue(":valor4", endereco);
        query.bindValue(":valor5", cpf);
        query.bindValue(":valor6", dataNasc);
        query.bindValue(":valor7", ehPf);
        query.bindValue(":valor8", id);
        if(!query.exec()){
            qDebug() << "erro query, update clientes";
        }

        db.close();
        emit clienteAtualizado();

        this->close();
    }
}

