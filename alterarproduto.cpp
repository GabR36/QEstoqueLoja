#include "alterarproduto.h"
#include "ui_alterarproduto.h"
#include "mainwindow.h"
#include "QSqlQuery"

AlterarProduto::AlterarProduto(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlterarProduto)
{
    ui->setupUi(this);
    ui->Ledit_AltNome->setText("teste");
}

AlterarProduto::~AlterarProduto()
{
    delete ui;
}

 void AlterarProduto::TrazerInfo(QString nome, QString desc, QString quant, QString preco, MainWindow *janela){
    ui->Ledit_AltNome->setText(nome);
    ui->Ledit_AltDesc->setText(desc);
    ui->Ledit_AltQuant->setText(quant);
    ui->Ledit_AltPreco->setText(preco);

    janelaPrincipal = janela;
}

void AlterarProduto::on_Btn_AltAceitar_accepted()
{
    QString nome = ui->Ledit_AltNome->text();
    QString desc = ui->Ledit_AltDesc->text();
    QString quant = ui->Ledit_AltQuant->text();
    QString preco = ui->Ledit_AltPreco->text();

    // alterar banco de dados
    if(!janelaPrincipal->db.open()){
        qDebug() << "erro ao abrir banco de dados. botao alterar->aceitar.";
    }
    QSqlQuery query;

    query.prepare("UPDATE produtos SET quantidade = :valor2, nome = :valor3, descricao = :valor4, preco = :valor5 WHERE id = :valor1");
    query.bindValue(":valor1", idAlt);
    query.bindValue(":valor2", quant);
    query.bindValue(":valor3", nome);
    query.bindValue(":valor4", desc);
    query.bindValue(":valor5", preco);
    if (query.exec()) {
        qDebug() << "Alteracao bem-sucedida!";
    } else {
        qDebug() << "Erro na alteracao: ";
    }
    // mostrar na tableview
    janelaPrincipal->atualizarTableview();
    QSqlDatabase::database().close();
}

