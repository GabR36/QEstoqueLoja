#include "inserircliente.h"
#include "ui_inserircliente.h"
#include <QMessageBox>
#include <QSqlQuery>

InserirCliente::InserirCliente(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InserirCliente)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool);
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
    // pegar os valores dos lineedits para colocar no banco de dados
    QString nome, email, telefone, endereco, cpf, dataNasc;
    nome = ui->Ledit_Nome->text();
    email = ui->Ledit_Email->text();
    telefone = ui->Ledit_Telefone->text();
    endereco = ui->Ledit_Cpf->text();
    cpf = ui->Ledit_Cpf->text();
    dataNasc = ui->Ledit_DataNascimento->text();

    // impedir nome de ser vazio
    if (nome == ""){
        QMessageBox::warning(this, "Erro", "Por favor, insira um nome.");
    }
    else{
        // alterar os valores no banco
        db.open();

        QSqlQuery query;
        query.prepare("INSERT INTO clientes (nome, email, telefone, endereco, cpf, data_nascimento) "
                      "VALUES (:valor1, :valor2, :valor3, :valor4, :valor5, :valor6)");
        query.bindValue(":valor1", nome);
        query.bindValue(":valor2", email);
        query.bindValue(":valor3", telefone);
        query.bindValue(":valor4", endereco);
        query.bindValue(":valor5", cpf);
        query.bindValue(":valor6", dataNasc);
        if(!query.exec()){
            qDebug() << "erro query, insert clientes";
        }

        db.close();

        this->close();
    }
}

