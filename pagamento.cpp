#include "pagamento.h"
#include "ui_pagamento.h"
#include <QSqlQuery>
#include <QMessageBox>
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

    // valores padrao
    ui->Ledit_Recebido->setText(totalGlobal);
    ui->Lbl_Troco->setText("0");
    ui->Ledit_Desconto->setText("0");
    ui->Ledit_Taxa->setText("0");
    ui->Lbl_TotalTaxa->setText(totalGlobal);

    // validador
    QDoubleValidator *validador = new QDoubleValidator();
    ui->Ledit_Taxa->setValidator(validador);
    ui->Ledit_Recebido->setValidator(validador);
    ui->Ledit_Desconto->setValidator(validador);
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
    QString taxa = ui->Ledit_Taxa->text();
    QString valor_final = ui->Lbl_TotalTaxa->text();
    QString desconto = ui->Ledit_Desconto->text();

    // validar line edits

    // desconto
    bool conversionOkDesconto;
    // tentar converter para float e ser menor ou igual ao valor total
    bool menorQueTotal = portugues.toFloat(desconto, &conversionOkDesconto) <= portugues.toFloat(totalGlobal);
    if (!menorQueTotal){
        conversionOkDesconto = false;
    }
    if (!conversionOkDesconto){
        // algo deu errado na conversao, desconto nao validado
        // inserir mensagem de erro e impedir insersao de venda
        QMessageBox::warning(this, "Erro", "Por favor, insira um desconto válido.");
        return;
    }

    // recebido
    qDebug() << "validar valor recebido";
    qDebug() << recebido;
    bool conversionOkRecebido;
    // testar se o recebido consegue ser convertido em float e se é maior ou igual ao total
    bool maiorQueTotal = portugues.toFloat(recebido, &conversionOkRecebido) >= portugues.toFloat(totalGlobal);
    if (!maiorQueTotal){
        // caso não seja maior ou igual que o total avalie como erro.
        conversionOkRecebido = false;
    }
    qDebug() << conversionOkRecebido;
    if (!conversionOkRecebido){
        // algo deu errado na conversao, recebido nao validado
        // inserir mensagem de erro e impedir insersao de venda
        QMessageBox::warning(this, "Erro", "Por favor, insira um valor recebido válido.");
        return;
    }

    // taxa
    qDebug() << "validar taxa";
    qDebug() << recebido;
    bool conversionOkTaxa;
    // testar se a taxa consegue ser converido em float
    portugues.toFloat(taxa, &conversionOkTaxa);
    qDebug() << conversionOkTaxa;
    if (!conversionOkTaxa){
        // algo deu errado na conversao, troco nao validado
        // inserir mensagem de erro e impedir insersao de venda
        QMessageBox::warning(this, "Erro", "Por favor, insira uma taxa válida.");
        return;
    }

    query.prepare("INSERT INTO vendas2 (cliente, total, data_hora, forma_pagamento, valor_recebido, troco, taxa, valor_final, desconto) VALUES (:valor1, :valor2, :valor3, :valor4, :valor5, :valor6, :valor7, :valor8, :valor9)");
    query.bindValue(":valor1", clienteGlobal);
    // precisa converter para notacao usa para inserir no banco de dados
    query.bindValue(":valor2", QString::number(portugues.toFloat(totalGlobal)));
    // inserir a data do dateedit
    query.bindValue(":valor3", dataGlobal);
    //
    query.bindValue(":valor4", forma_pagamento);
    // precisa converter para notacao usa para inserir no banco de dados
    query.bindValue(":valor5", QString::number(portugues.toFloat(recebido)));
    query.bindValue(":valor6", QString::number(portugues.toFloat(troco)));
    query.bindValue(":valor7", QString::number(portugues.toFloat(taxa)));
    query.bindValue(":valor8", QString::number(portugues.toFloat(valor_final)));
    query.bindValue(":valor9", QString::number(portugues.toFloat(desconto)));

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
        // precisa converter para notacao usa para inserir no banco de dados
        query.bindValue(":valor2", QString::number(portugues.toInt(rowdata[1].toString())));
        query.bindValue(":valor3", QString::number(portugues.toFloat(rowdata[3].toString())));
        query.bindValue(":valor4", idVenda);
        if (query.exec()) {
            qDebug() << "Inserção prod_vendidos bem-sucedida!";
        } else {
            qDebug() << "Erro na inserção prod_vendidos: ";
        }
        query.prepare("UPDATE produtos SET quantidade = quantidade - :valor2 WHERE id = :valor1");
        query.bindValue(":valor1", rowdata[0]);
        // precisa converter para notacao usa para inserir no banco de dados
        query.bindValue(":valor2", QString::number(portugues.toInt(rowdata[1].toString())));
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
    float troco = portugues.toFloat(dinRecebido) - portugues.toFloat(totalGlobal);

    ui->Lbl_Troco->setText(portugues.toString(troco));

}


void pagamento::on_CBox_FormaPagamento_activated(int index)
{
    // mostrar ou esconder campos relacionados ao troco
    // a depender da forma dinheiro ser selecionada
    QString taxaDebito  = "3";
    QString taxaCredito = "4";
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
        ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(taxaCredito, "0")));
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
        ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(taxaDebito, "0")));
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
    QString novaTaxa = ui->Ledit_Taxa->text();
    QString desconto = ui->Ledit_Desconto->text();
    ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(novaTaxa, desconto)));
}

float pagamento::obterValorFinal(QString taxa, QString desconto){
    float valorFinal = portugues.toFloat(totalGlobal) * (1 + portugues.toFloat(taxa)/100) - portugues.toFloat(desconto);
    return valorFinal;
}

void pagamento::on_Ledit_Desconto_textChanged(const QString &arg1)
{
    QString novaTaxa = ui->Ledit_Taxa->text();
    QString desconto = ui->Ledit_Desconto->text();
    ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(novaTaxa, desconto)));
}

