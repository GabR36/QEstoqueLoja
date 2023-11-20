#include "alterarproduto.h"
#include "ui_alterarproduto.h"
#include "mainwindow.h"
#include "QSqlQuery"

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
    ui->Ledit_AltDesc->setText(desc);
    ui->Ledit_AltQuant->setText(quant);
    ui->Ledit_AltPreco->setText(preco);
}

void AlterarProduto::on_Btn_AltAceitar_accepted()
{
    QString desc = ui->Ledit_AltDesc->text();
    QString quant = ui->Ledit_AltQuant->text();
    QString preco = ui->Ledit_AltPreco->text();

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

