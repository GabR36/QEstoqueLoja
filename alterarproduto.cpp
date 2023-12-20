#include "alterarproduto.h"
#include "ui_alterarproduto.h"
#include "mainwindow.h"
#include "QSqlQuery"
#include <QMessageBox>

AlterarProduto::AlterarProduto(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlterarProduto)
{
    ui->setupUi(this);
}

AlterarProduto::~AlterarProduto()
{
    delete ui;
}

 void AlterarProduto::TrazerInfo(QString desc, QString quant, QString preco){
    descAlt = desc;
    quantAlt = quant;
    precoAlt = preco;
    ui->Ledit_AltDesc->setText(desc);
    ui->Ledit_AltQuant->setText(quant);
    ui->Ledit_AltPreco->setText(preco);
}

void AlterarProduto::on_Btn_AltAceitar_accepted()
{
    QString desc = ui->Ledit_AltDesc->text();
    QString quant = ui->Ledit_AltQuant->text();
    QString preco = ui->Ledit_AltPreco->text();

    // Substitua ',' por '.' se necessário
    preco.replace(',', '.');

    // Converta o texto para um número
    bool conversionOk;
    double price = preco.toDouble(&conversionOk);

    // Verifique se a conversão foi bem-sucedida e se o preço é maior que zero
    if (conversionOk && price >= 0)
    {
        // Cria uma mensagem de confirmação
        QMessageBox::StandardButton resposta;
        resposta = QMessageBox::question(
            nullptr,
            "Confirmação",
            "Tem certeza que deseja alterar o produto:\n\n"
            "id: " + idAlt + "\n"
            "Descrição: " + descAlt + "\n"
            "Quantidade: " + quantAlt + "\n"
            "Preço: " + precoAlt + "\n\n"
            "Para: \n\n"
            "Descrição: " + desc + "\n"
            "Quantidade: " + quant + "\n"
            "Preço: " + preco + "\n\n",
            QMessageBox::Yes | QMessageBox::No
        );
        // Verifica a resposta do usuário
        if (resposta == QMessageBox::Yes) {
            // alterar banco de dados
            if(!janelaPrincipal->db.open()){
                qDebug() << "erro ao abrir banco de dados. botao alterar->aceitar.";
            }
            QSqlQuery query;

            query.prepare("UPDATE produtos SET quantidade = :valor2, descricao = :valor3, preco = :valor4 WHERE id = :valor1");
            query.bindValue(":valor1", idAlt);
            query.bindValue(":valor2", quant);
            query.bindValue(":valor3", desc);
            query.bindValue(":valor4", preco);
            if (query.exec()) {
                qDebug() << "Alteracao bem-sucedida!";
            } else {
                qDebug() << "Erro na alteracao: ";
            }
            // mostrar na tableview
            janelaPrincipal->atualizarTableview();
            QSqlDatabase::database().close();
        }
        else {
            // O usuário escolheu não alterar o produto
            qDebug() << "A alteraçao do produto foi cancelada.";
        }
    }
    else
    {
        // Exiba uma mensagem de erro se o preço não for válido
        QMessageBox::warning(this, "Erro", "Por favor, insira um preço válido.");
    }
}

