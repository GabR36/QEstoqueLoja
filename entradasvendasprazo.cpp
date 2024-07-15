#include "entradasvendasprazo.h"
#include "ui_entradasvendasprazo.h"
#include <QDebug>
#include <QSqlQuery>
#include <QDateTime>
#include <QMessageBox>
#include "pagamentoaprazo.h"
#include <QDoubleValidator>


EntradasVendasPrazo::EntradasVendasPrazo(QWidget *parent, QString id_venda)
    : QDialog(parent)
    , ui(new Ui::EntradasVendasPrazo)
{
    ui->setupUi(this);
    idVenda = id_venda;
    ui->label->setText(idVenda);

    // validador
    QDoubleValidator *validador = new QDoubleValidator(0.0, 9999.99, 2);
    ui->ledit_AddValor->setValidator(validador);

    // ui->tw_Entradas.setWindowTitle("Exemplo de QTableWidget");
    // ui->tw_Entradas.resize(400, 300);
    if(db.open()){
        qDebug() << "banco de dados abridokkl ";
    }
    QSqlQuery query;
    QDateTime data_Venda;
    query.prepare("SELECT valor_final, data_hora, cliente FROM vendas2 WHERE id = :id_venda");
    query.bindValue(":id_venda", idVenda);
    if (query.exec()) {
        while (query.next()) {
            valor_Venda = query.value("valor_final").toFloat();
            data_Venda = query.value("data_hora").toDateTime();
            QString cliente = query.value("cliente").toString();

            valorVenda = portugues.toString(valor_Venda, 'f', 2);
            dataHoraVenda = portugues.toString(data_Venda, "dd/MM/yyyy");
            clienteVenda = cliente;
        }
    }
    atualizarTabelaPag();
    ui->tview_Entradas->setModel(modeloEntradas);  // Atualize o widget para tableView
    modeloEntradas->setHeaderData(0, Qt::Horizontal, tr("Total"));
    modeloEntradas->setHeaderData(1, Qt::Horizontal, tr("Data e Hora"));
    modeloEntradas->setHeaderData(2, Qt::Horizontal, tr("Forma de Pagamento"));
    modeloEntradas->setHeaderData(3, Qt::Horizontal, tr("Valor Final"));
    modeloEntradas->setHeaderData(4, Qt::Horizontal, tr("Troco"));
    modeloEntradas->setHeaderData(5, Qt::Horizontal, tr("Taxa"));
    modeloEntradas->setHeaderData(6, Qt::Horizontal, tr("Valor Recebido"));
    modeloEntradas->setHeaderData(7, Qt::Horizontal, tr("Desconto"));
    ui->tview_Entradas->setColumnWidth(0,100);
    ui->tview_Entradas->setColumnWidth(1,150);
    ui->tview_Entradas->setColumnWidth(2,160);

    ui->label_2->setText("Valor Inicial da Venda: " + valorVenda);
    ui->label_3->setText("Data Inicial da Venda: "+ dataHoraVenda);
    ui->label_4->setText("Cliente: "+clienteVenda);


    db.close();
}

EntradasVendasPrazo::~EntradasVendasPrazo()
{
    delete ui;


}
void EntradasVendasPrazo::atualizarTabelaPag(){
    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados";
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT total, data_hora, forma_pagamento, valor_final, troco, taxa, valor_recebido, desconto FROM entradas_vendas WHERE id_venda = :valoridvenda");
    query.bindValue(":valoridvenda", idVenda);

    if (!query.exec()) {
        qDebug() << "Erro ao executar consulta";
        db.close();
        return;
    }

    // Atualizar o modelo para a QTableView
    modeloEntradas->setQuery(query);


    float valorDevido = valor_Venda;
    for (int i = 0; i < modeloEntradas->rowCount(); ++i) {
        float valorRecebido = modeloEntradas->data(modeloEntradas->index(i, 0)).toFloat();
        valorDevido -= valorRecebido;
    }


    ui->label_5->setText("Devendo: R$" + portugues.toString(valorDevido, 'f', 2));
    if (valorDevido > 0) {
        ui->label_5->setStyleSheet("color: red");
    } if(valorDevido <= 0){
        ui->label_5->setStyleSheet("color: green");
        query.prepare("UPDATE vendas2 SET esta_pago = 1 WHERE id = :valoridvenda");
        query.bindValue(":valoridvenda", idVenda);
        if(query.exec()){
            qDebug() << "venda a prazo paga";
        }
    }
    valorDevidoGlobal = portugues.toFloat(portugues.toString(valorDevido, 'f', 2));

    db.close();
}


void EntradasVendasPrazo::on_btn_AddValor_clicked()
{
    float valorInserido = portugues.toFloat(ui->ledit_AddValor->text());
    // se tiver algo a dever e o valor informado for menor ou igual ao valor devido
    if ((valorDevidoGlobal > 0) && (valorInserido <= valorDevidoGlobal)) {

        pagamentoAPrazo *pgmntPrazo= new pagamentoAPrazo(idVenda, portugues.toString(valorInserido, 'f', 2), clienteVenda, portugues.toString(QDateTime::currentDateTime(), "yyyy-MM-dd hh:mm:ss"));
        connect(pgmntPrazo, &QObject::destroyed, this, &EntradasVendasPrazo::onPgmntFechado);
        pgmntPrazo->show();
    }

}

void EntradasVendasPrazo::onPgmntFechado(){
    qDebug() << "atualizarTAbelaPag?";
    atualizarTabelaPag();

    if(valorDevidoGlobal <= 0) {
        ui->btn_AddValor->setEnabled(false);
    }
    ui->ledit_AddValor->clear();

}

