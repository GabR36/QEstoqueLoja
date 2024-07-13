#include "entradasvendasprazo.h"
#include "ui_entradasvendasprazo.h"
#include <QDebug>
#include <QSqlQuery>
#include <QDateTime>
#include <QMessageBox>
#include "pagamentoaprazo.h"


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
            valor_Venda = query.value("valor_final").toFloat();
            data_Venda = query.value("data_hora").toDateTime();
            QString cliente = query.value("cliente").toString();

            valorVenda = portugues.toString(valor_Venda, 'f', 2);
            dataHoraVenda = portugues.toString(data_Venda, "dd/MM/yyyy hh:mm:ss");
            clienteVenda = cliente;
        }
    }
    modeloEntradas->setHeaderData(0, Qt::Horizontal, tr("Total"));
    modeloEntradas->setHeaderData(1, Qt::Horizontal, tr("Data e Hora"));

    ui->tview_Entradas->setModel(modeloEntradas);  // Atualize o widget para tableView

    ui->label_2->setText("O valor da venda é: " + valorVenda);
    ui->label_3->setText("Data: "+ dataHoraVenda);
    ui->label_4->setText("cliente: "+clienteVenda);


    db.close();
    ui->tview_Entradas->setColumnWidth(0,400);
    ui->tview_Entradas->setColumnWidth(1,200);

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

    // Atualizar o modelo para a QTableView
    modeloEntradas->setQuery(query);


    float valorDevido = valor_Venda;
    for (int i = 0; i < modeloEntradas->rowCount(); ++i) {
        float valorRecebido = modeloEntradas->data(modeloEntradas->index(i, 0)).toFloat();
        valorDevido -= valorRecebido;
    }

    ui->label_5->setText("A Dever: R$" + portugues.toString(valorDevido, 'f', 2));
    if (valorDevido > 0) {
        ui->label_5->setStyleSheet("color: red");
    } else {
        ui->label_5->setStyleSheet("color: green");
    }
    valorDevidoGlobal = valorDevido;

    db.close();
}


void EntradasVendasPrazo::on_btn_AddValor_clicked()
{
    if (valorDevidoGlobal > 0) {

        pagamentoAPrazo *pgmntPrazo= new pagamentoAPrazo("100", "juliano", "10/10/2010");
        pgmntPrazo->show();

        if (db.open()) {
            QSqlQuery query;
            query.prepare("INSERT INTO entradas_vendas(id_venda, data_hora, total) VALUES (:valoridvenda, :valordatahora, :valorrecebido)");
            query.bindValue(":valoridvenda", idVenda);
            query.bindValue(":valordatahora", portugues.toString(QDateTime::currentDateTime(), "yyyy-MM-dd hh:mm:ss"));
            query.bindValue(":valorrecebido", ui->ledit_AddValor->text());

            if (query.exec()) {
                qDebug() << "Inserção realizada com sucesso";
                atualizarTabelaPag();
            } else {
                qDebug() << "Erro ao inserir valor:" ;
            }
        } if(valorDevidoGlobal <= 0) {
            ui->btn_AddValor->setEnabled(false);
        }
        db.close();
    }

}

