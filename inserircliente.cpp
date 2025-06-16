#include "inserircliente.h"
#include "ui_inserircliente.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QRegularExpression>
#include <QSqlError>
#include <QDebug>

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
    // Pegar os valores dos campos de entrada
    QString nome = ui->Ledit_Nome->text().trimmed();
    QString email = ui->Ledit_Email->text().trimmed();
    QString telefone = ui->Ledit_Telefone->text().trimmed();
    QString endereco = ui->Ledit_Endereco->text().trimmed();
    QString cpf = ui->Ledit_Cpf->text().trimmed();
    QString dataNasc = ui->Ledit_DataNascimento->text().trimmed();

    bool ehPf = ui->RadioB_Pfisica->isChecked(); // Define o tipo de cliente (Pessoa Física ou Jurídica)

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

    // Validação da Data de Nascimento (Apenas se o campo não estiver vazio)
    if (!dataNasc.isEmpty()) {
        QRegularExpression dataRegex(R"(^(0[1-9]|[12][0-9]|3[01])/(0[1-9]|1[0-2])/\d{4}$)");
        if (!dataRegex.match(dataNasc).hasMatch()) {
            QMessageBox::warning(this, "Erro", "Data de nascimento inválida! Use o formato dd/MM/yyyy.");
            return;
        }
    }

    // Inserir no banco de dados
    if (!db.open()) {
        QMessageBox::warning(this, "Erro", "Erro ao conectar ao banco de dados.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO clientes (nome, email, telefone, endereco, cpf, data_nascimento, eh_pf) "
                  "VALUES (:nome, :email, :telefone, :endereco, :cpf, :dataNasc, :ehPf)");
    query.bindValue(":nome", nome);
    query.bindValue(":email", email.isEmpty() ? QVariant(QVariant::String) : email);
    query.bindValue(":telefone", telefone.isEmpty() ? QVariant(QVariant::String) : telefone);
    query.bindValue(":endereco", endereco.isEmpty() ? QVariant(QVariant::String) : endereco);
    query.bindValue(":cpf", cpf.isEmpty() ? QVariant(QVariant::String) : cpf);
    query.bindValue(":dataNasc", dataNasc.isEmpty() ? QVariant(QVariant::String) : dataNasc);
    query.bindValue(":ehPf", ehPf);

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
