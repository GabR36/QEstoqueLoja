#include "alterarcliente.h"
#include "ui_alterarcliente.h"
#include <QSqlQuery>

AlterarCliente::AlterarCliente(QWidget *parent, QString id)
    : QWidget(parent)
    , ui(new Ui::AlterarCliente)
{
    ui->setupUi(this);

    qDebug() << "id alterar cliente: " + id;

    // obter as informações do cliente para colocar nos linedits
    if(!db.open()){
        qDebug() << "db não abriu, janela alterar cliente";
    }
    QSqlQuery query;
    query.prepare("SELECT nome, email, telefone, endereco, cpf, data_nascimento FROM clientes "
                  "WHERE id = :valor1");
    query.bindValue(":valor1", id);
    if(!query.exec()){
        qDebug() << "Query erro, alterar cliente.";
    }
    QString nome, email, telefone, endereco, cpf, dataNasc;
    query.next();
    nome = query.value(0).toString();
    email = query.value(1).toString();
    telefone = query.value(2).toString();
    endereco = query.value(3).toString();
    cpf = query.value(4).toString();
    dataNasc = query.value(5).toString();
    qDebug() << "valores alterar cliente: " + nome + email + telefone + endereco + cpf + dataNasc;

    // colocar os valores nos lineedits
    ui->Ledit_Cpf->setText(cpf);
    ui->Ledit_DataNascimento->setText(dataNasc);
    ui->Ledit_Email->setText(email);
    ui->Ledit_Endereco->setText(endereco);
    ui->Ledit_Nome->setText(nome);
    ui->Ledit_Telefone->setText(telefone);
}

AlterarCliente::~AlterarCliente()
{
    delete ui;
}
