#include "pagamento.h"
#include "ui_pagamento.h"
#include <QSqlQuery>

pagamento::pagamento(QString total, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::pagamento)
{
    ui->setupUi(this);

    totalGlobal = total;
    ui->Lbl_ResumoTotal->setText(total);

    ui->Ledit_Recebido->setFocus();
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

    query.prepare("INSERT INTO vendas2 (cliente, total, data_hora, forma_pagamento, valor_recebido, troco) VALUES (:valor1, :valor2, :valor3, :valor4, :valor5, :valor6)");
    query.bindValue(":valor1", cliente);
    query.bindValue(":valor2", totalGlobal);
    // inserir a data do dateedit
    query.bindValue(":valor3", data);
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
    if (index == 0){
        ui->Lbl_Troco->show();
        ui->label_2->show();
        ui->label_3->show();
        ui->Ledit_Recebido->show();
    }
    else {
        ui->Lbl_Troco->hide();
        ui->label_2->hide();
        ui->label_3->hide();
        ui->Ledit_Recebido->hide();
    }
}

