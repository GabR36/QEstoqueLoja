#include "pagamento.h"
#include "ui_pagamento.h"
#include <QSqlQuery>

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

    // esconder os campos nao relativos a forma dinheiro (taxa e valor apos taxa)
    ui->label_8->hide();
    ui->Ledit_Taxa->hide();
    ui->label_10->hide();
    ui->Lbl_TotalTaxa->hide();
}

pagamento::~pagamento()
{
    delete ui;
}

void pagamento::on_buttonBox_accepted()
{
    // inserir a venda

    // adicionar ao banco de dados
    if(!janelaVenda->db.open()){
        qDebug() << "erro ao abrir banco de dados. botao aceitar venda.";
    }
    QSqlQuery query;

    QString troco = ui->Lbl_Troco->text();
    QString recebido = ui->Ledit_Recebido->text();
    QString forma_pagamento = ui->CBox_FormaPagamento->currentText();

    // se a forma de pagamento não for dinheiro atribua o valor total para
    // o valor recebido e 0 para o troco
    if (ui->CBox_FormaPagamento->currentIndex() != 0){
        troco = "0";
        recebido = totalGlobal;
    }

    query.prepare("INSERT INTO vendas2 (cliente, total, data_hora, forma_pagamento, valor_recebido, troco) VALUES (:valor1, :valor2, :valor3, :valor4, :valor5, :valor6)");
    query.bindValue(":valor1", clienteGlobal);
    query.bindValue(":valor2", totalGlobal);
    // inserir a data do dateedit
    query.bindValue(":valor3", dataGlobal);
    //
    query.bindValue(":valor4", forma_pagamento);
    query.bindValue(":valor5", recebido);
    query.bindValue(":valor6", troco);

    QString idVenda;
    if (query.exec()) {
        idVenda = query.lastInsertId().toString();
        qDebug() << "Inserção bem-sucedida!";
    } else {
        qDebug() << "Erro na inserção: ";
    }
    // inserir os produtos da venda

    // adicionar ao banco de dados
    for (const QList<QVariant> &rowdata : rowDataList) {
        query.prepare("INSERT INTO produtos_vendidos (id_produto, quantidade, preco_vendido, id_venda) VALUES (:valor1, :valor2, :valor3, :valor4)");
        query.bindValue(":valor1", rowdata[0]);
        query.bindValue(":valor2", rowdata[1]);
        query.bindValue(":valor3", rowdata[3]);
        query.bindValue(":valor4", idVenda);
        if (query.exec()) {
            qDebug() << "Inserção prod_vendidos bem-sucedida!";
        } else {
            qDebug() << "Erro na inserção prod_vendidos: ";
        }
        query.prepare("UPDATE produtos SET quantidade = quantidade - :valor2 WHERE id = :valor1");
        query.bindValue(":valor1", rowdata[0]);
        query.bindValue(":valor2", rowdata[1]);
        if (query.exec()) {
            qDebug() << "update quantidade bem-sucedida!";
        } else {
            qDebug() << "Erro na update quantidade: ";
        }
    }
    janelaVenda->db.close();
    janelaVenda->janelaVenda->atualizarTabelas();
    janelaVenda->janelaPrincipal->atualizarTableview();

    // fechar as janelas
    this->close();
    janelaVenda->close();
}


void pagamento::on_Ledit_Recebido_textChanged(const QString &arg1)
{
    QString dinRecebido = ui->Ledit_Recebido->text();
    float troco = dinRecebido.toFloat() - totalGlobal.toFloat();

    ui->Lbl_Troco->setText(QString::number(troco));

}


void pagamento::on_CBox_FormaPagamento_activated(int index)
{
    // mostrar ou esconder campos relacionados ao troco
    // a depender da forma dinheiro ser selecionada
    switch (index) {
    case 0:
        ui->Lbl_Troco->show();
        ui->label_2->show();
        ui->label_3->show();
        ui->Ledit_Recebido->show();
        ui->label_8->hide();
        ui->Ledit_Taxa->hide();
        ui->label_10->hide();
        ui->Lbl_TotalTaxa->hide();
        break;
    case 2:
        ui->Lbl_Troco->hide();
        ui->label_2->hide();
        ui->label_3->hide();
        ui->Ledit_Recebido->hide();
        ui->label_8->show();
        ui->Ledit_Taxa->show();
        ui->label_10->show();
        ui->Lbl_TotalTaxa->show();
        break;
    case 3:
        ui->Lbl_Troco->hide();
        ui->label_2->hide();
        ui->label_3->hide();
        ui->Ledit_Recebido->hide();
        ui->label_8->show();
        ui->Ledit_Taxa->show();
        ui->label_10->show();
        ui->Lbl_TotalTaxa->show();
        break;
    default:
        ui->Lbl_Troco->hide();
        ui->label_2->hide();
        ui->label_3->hide();
        ui->Ledit_Recebido->hide();
        ui->label_8->hide();
        ui->Ledit_Taxa->hide();
        ui->label_10->hide();
        ui->Lbl_TotalTaxa->hide();
        break;
    }
}

