#include "entradasvendasprazo.h"
#include "ui_entradasvendasprazo.h"
#include <QDebug>
#include <QSqlQuery>
#include <QDateTime>
EntradasVendasPrazo::EntradasVendasPrazo(QWidget *parent, QString id_venda)
    : QDialog(parent)
    , ui(new Ui::EntradasVendasPrazo)
{
    ui->setupUi(this);
    idVenda = id_venda;
    ui->label->setText(idVenda);

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
            float valor_Venda = query.value("valor_final").toFloat();
            data_Venda = query.value("data_hora").toDateTime();
            QString cliente = query.value("cliente").toString();

            valorVenda = portugues.toString(valor_Venda, 'f', 2);
            dataHoraVenda = portugues.toString(data_Venda, "dd/MM/yyyy hh:mm:ss");
            clienteVenda = cliente;

        }

    }

    // QStringList headers;
    // headers << "Valor" << "Data";
    // ui->tw_Entradas->setColumnCount(headers.size());
    // ui->tw_Entradas->setHorizontalHeaderLabels(headers);

    ui->label_2->setText("O valor da venda é: " + valorVenda);
    ui->label_3->setText("Data: "+ dataHoraVenda);
    ui->label_4->setText("cliente: "+clienteVenda);


    db.close();
    atualizarTabelaPag();

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
    query.prepare("SELECT total, data_hora FROM entradas_vendas WHERE id_venda = :valoridvenda");
    query.bindValue(":valoridvenda", idVenda);

    if (!query.exec()) {
        qDebug() << "Erro ao executar consulta";
        db.close();
        return;
    }

    // Limpar a tabela antes de atualizar
    ui->tw_Entradas->setRowCount(0);

    int row = 0;
    while (query.next()) {
        QString valorRecebido = query.value("total").toString();
        QDateTime dataRecebido = query.value("data_hora").toDateTime();

        ui->tw_Entradas->insertRow(row);
        ui->tw_Entradas->setItem(row, 0, new QTableWidgetItem(valorRecebido));
        ui->tw_Entradas->setItem(row, 1, new QTableWidgetItem(portugues.toString(dataRecebido, "dd/MM/yyyy hh:mm:ss")));
        row++;
    }

    db.close();
}

void EntradasVendasPrazo::on_btn_AddValor_clicked()
{
    if(db.open()){
        qDebug() << "banco de dados aberto botao add ";
    }
    QSqlQuery query;
    QDateTime dataInglesAgora;
    dataInglesAgora.currentDateTime();
    query.prepare("INSERT INTO entradas_vendas(id_venda,data_hora,total) VALUES (:valoridvenda, :valordatahora, :valorrecebido)");
    query.bindValue(":valoridvenda", idVenda);
    query.bindValue(":valordatahora", portugues.toString(QDateTime::currentDateTime(), "yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":valorrecebido", ui->ledit_AddValor->text());
    if(query.exec()){
        while(query.next()){

        }
        qDebug() << "ibnserção entreada";
    }


    db.close();
    atualizarTabelaPag();

}

