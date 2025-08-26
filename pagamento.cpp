#include "pagamento.h"
#include "ui_pagamento.h"
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QSqlQuery>
#include <QDoubleValidator>

pagamento::pagamento(QString total, QString cliente, QString data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::pagamento)
{
    ui->setupUi(this);

    totalGlobal = total;
    ui->Lbl_ResumoTotal->setText(total);
    clienteGlobal = cliente;
    ui->Lbl_ResumoCliente->setText(cliente);
    dataGlobal = data;
    ui->Lbl_ResumoData->setText(data);

    ui->Ledit_Recebido->setFocus();

    // esconder os campos nao relativos a forma dinheiro (taxa)
    ui->label_8->hide();
    ui->Ledit_Taxa->hide();

    ui->FrameNF->setVisible(false);
    ui->Ledit_NNF->setVisible(false);
    ui->Lbl_NNF->setVisible(false);


    // valores padrao
    ui->Ledit_Recebido->setText(totalGlobal);
    ui->Lbl_Troco->setText("0");
    ui->Ledit_Desconto->setText("0");
    ui->Ledit_Taxa->setText("0");
    ui->Lbl_TotalTaxa->setText(totalGlobal);

    // validador
    QDoubleValidator *validador = new QDoubleValidator(0.0, 9999.99, 2);
    ui->Ledit_Taxa->setValidator(validador);
    ui->Ledit_Recebido->setValidator(validador);
    ui->Ledit_Desconto->setValidator(validador);
    QIntValidator *validadorInt = new QIntValidator();
    ui->Ledit_NNF->setValidator(validadorInt);
}

pagamento::~pagamento()
{
    delete ui;
}

void pagamento::on_buttonBox_accepted()
{
    terminarPagamento();
}


void pagamento::on_Ledit_Recebido_textChanged(const QString &arg1)
{
    QString dinRecebido = ui->Ledit_Recebido->text();
    QString valorFinal = ui->Lbl_TotalTaxa->text();
    float troco = portugues.toFloat(dinRecebido) - portugues.toFloat(valorFinal);

    ui->Lbl_Troco->setText(portugues.toString(troco, 'f', 2));

}


void pagamento::on_CBox_FormaPagamento_activated(int index)
{
    // mostrar ou esconder campos relacionados ao troco
    // a depender da forma dinheiro ser selecionada

    // pegar os valores de taxas padrao das configuracoes
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. pagamento combobox";
    }
    QSqlQuery query;

    QString taxaDebito  = "3";
    QString taxaCredito = "4";
    if (query.exec("SELECT value FROM config WHERE key = 'taxa_debito'")){
        while (query.next()) {
            taxaDebito = portugues.toString(query.value(0).toFloat());
        }
    }
    if (query.exec("SELECT value FROM config WHERE key = 'taxa_credito'")){
        while (query.next()) {
            taxaCredito = portugues.toString(query.value(0).toFloat());
        }
    }
    //

    switch (index) {
    case 0:
        // dinheiro
        ui->Lbl_Troco->show();
        ui->label_2->show();
        ui->label_3->show();
        ui->Ledit_Recebido->show();
        ui->label_8->hide();
        ui->Ledit_Taxa->hide();

        // valores padrao
        ui->Ledit_Recebido->setText(totalGlobal);
        ui->Lbl_Troco->setText("0");
        ui->Ledit_Desconto->setText("0");
        ui->Ledit_Taxa->setText("0");
        ui->Lbl_TotalTaxa->setText(totalGlobal);
        break;
    case 2:
        // credito
        ui->Lbl_Troco->hide();
        ui->label_2->hide();
        ui->label_3->hide();
        ui->Ledit_Recebido->hide();
        ui->label_8->show();
        ui->Ledit_Taxa->show();

        // valores padrao
        ui->Ledit_Recebido->setText(totalGlobal);
        ui->Lbl_Troco->setText("0");
        ui->Ledit_Desconto->setText("0");
        ui->Ledit_Taxa->setText(taxaCredito);
        ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(taxaCredito, "0"), 'f', 2));
        break;
    case 3:
        // debito
        ui->Lbl_Troco->hide();
        ui->label_2->hide();
        ui->label_3->hide();
        ui->Ledit_Recebido->hide();
        ui->label_8->show();
        ui->Ledit_Taxa->show();

        // valores padrao
        ui->Ledit_Recebido->setText(totalGlobal);
        ui->Lbl_Troco->setText("0");
        ui->Ledit_Desconto->setText("0");
        ui->Ledit_Taxa->setText(taxaDebito);
        ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(taxaDebito, "0"), 'f', 2));
        break;
    default:
        ui->Lbl_Troco->hide();
        ui->label_2->hide();
        ui->label_3->hide();
        ui->Ledit_Recebido->hide();
        ui->label_8->hide();
        ui->Ledit_Taxa->hide();

        // valores padrao
        ui->Ledit_Recebido->setText(totalGlobal);
        ui->Lbl_Troco->setText("0");
        ui->Ledit_Desconto->setText("0");
        ui->Ledit_Taxa->setText("0");
        ui->Lbl_TotalTaxa->setText(totalGlobal);
        break;
    }
}


void pagamento::on_Ledit_Taxa_textChanged(const QString &arg1)
{
    descontoTaxa();
}

float pagamento::obterValorFinal(QString taxa, QString desconto){
    float valorFinal = (portugues.toFloat(totalGlobal) - portugues.toFloat(desconto)) * (1 + portugues.toFloat(taxa)/100);
    return valorFinal;
}

void pagamento::on_Ledit_Desconto_textChanged(const QString &arg1)
{
    descontoTaxa();
}

void pagamento::descontoTaxa(){
    QString novaTaxa = ui->Ledit_Taxa->text();
    QString descontoInicial = ui->Ledit_Desconto->text();
    QString desconto;
    if (ui->CheckPorcentagem->isChecked()){
        desconto = portugues.toString((portugues.toFloat(descontoInicial)/100)*portugues.toFloat(totalGlobal));
    }
    else{
        desconto = descontoInicial;
    }
    QString valorFinal = portugues.toString(obterValorFinal(novaTaxa, desconto), 'f', 2);
    ui->Lbl_TotalTaxa->setText(valorFinal);
    // o valor final influencia os campos recebido, portanto modificacoes nele devem afetar
    // os campos influenciados por ele

    // o valor recebido deve ser o mesmo que o valor final, pois ao alterar o valor final
    // o dinheiro recebido não será mais o mesmo do valor final anterior
    ui->Ledit_Recebido->setText(valorFinal);

    // o troco será zero portanto
    ui->Lbl_Troco->setText("0");
}

void pagamento::terminarPagamento()
{

}


void pagamento::on_CheckPorcentagem_stateChanged(int arg1)
{
    descontoTaxa();
}


void pagamento::on_CBox_ModeloEmit_currentIndexChanged(int index)
{
    if(index == 2){
        ui->RadioBtn_EmitNfApenas->setVisible(false);
        ui->RadioBtn_EmitNfTodos->setVisible(false);
    }else{
        ui->RadioBtn_EmitNfApenas->setVisible(true);
        ui->RadioBtn_EmitNfTodos->setVisible(true);
    }
}

