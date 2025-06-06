#include "pagamentovenda.h"
#include "subclass/waitdialog.h"
#include <QTimer>
pagamentoVenda::pagamentoVenda(QList<QList<QVariant>> listaProdutos, QString total, QString cliente, QString data, int idCliente, QWidget *parent)
    : pagamento(total, cliente, data, parent)
{
    rowDataList = listaProdutos;
    this->idCliente = idCliente;
    //qDebug() << QDir::currentPath();
    connect(this, &pagamentoVenda::gerarEnviarNf, &nota, &NfceVenda::onReqGerarEnviar);
    connect(&nota, &NfceVenda::retWSChange, this, &pagamentoVenda::onRetWSChange);
    connect(&nota, &NfceVenda::errorOccurred, this, &pagamentoVenda::onErrorOccurred);
    connect(&nota, &NfceVenda::retStatusServico, this, &pagamentoVenda::onRetStatusServico);



}
void pagamentoVenda::onErrorOccurred(const QString &error){
    if (waitDialog) {
        waitDialog->setMessage(error);
    }
   // erroNf = error;
}

void pagamentoVenda::onRetWSChange(const QString &webServices){
    if (waitDialog) {
        waitDialog->setMessage(webServices);
    }

}

void pagamentoVenda::onRetStatusServico(const QString &status){
    if (waitDialog) {
        waitDialog->setMessage(status);
    }
}
void pagamentoVenda::verificarErroNf(const CppNFe *cppnfe){
    if (cppnfe->notafiscal->retorno->protNFe->items->count() > 0)
    {
        if ((cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat() == 100) ||
            (cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat() == 150))
        {
            waitDialog->setMessage("Sucesso!\n Status:" + QString::number(cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat()));
            waitDialog->allowClose();
            QTimer::singleShot(2000, waitDialog, &QDialog::close); //fecha depois de 2 segundos
        }else{

            QString msg = "ERRO:\n" + cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_xMotivo();
            //waitDialog->allowClose();
            waitDialog->setMessage(msg);
            waitDialog->allowClose();
            //QTimer::singleShot(2000, waitDialog, &QDialog::close);
        }
    } else{

    }
}

void pagamentoVenda::terminarPagamento(){
    QDateTime dataIngles = portugues.toDateTime(dataGlobal, "dd-MM-yyyy hh:mm:ss");
    // inserir a venda

    // adicionar ao banco de dados
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao aceitar venda.";
    }
    QSqlQuery query;

    QString troco = ui->Lbl_Troco->text();
    QString recebido = ui->Ledit_Recebido->text();
    QString forma_pagamento = ui->CBox_FormaPagamento->currentText();
    QString taxa = ui->Ledit_Taxa->text();
    QString valor_final = ui->Lbl_TotalTaxa->text();
    QString descontoInicial = ui->Ledit_Desconto->text();
    QString desconto;
    if (ui->CheckPorcentagem->isChecked()){
        desconto = portugues.toString((portugues.toFloat(descontoInicial)/100)*portugues.toFloat(totalGlobal));
    }
    else{
        desconto = descontoInicial;
    }
    // validar line edits

    // desconto
    bool conversionOkDesconto;
    // tentar converter para float e ser menor ou igual ao valor final
    qDebug() << "validar valor desconto";
    qDebug() << desconto;
    bool menorQueTotal = portugues.toFloat(desconto, &conversionOkDesconto) <= portugues.toFloat(totalGlobal);
    qDebug() << conversionOkDesconto;
    if (!menorQueTotal){
        conversionOkDesconto = false;
    }
    if(forma_pagamento == "Prazo" && idCliente == 1){
        QMessageBox::warning(this,"Erro", "Especifique um cliente diferente para vender à prazo!");
        return;
    };
    qDebug() << conversionOkDesconto;
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
    // testar se o recebido consegue ser convertido em float e se é maior ou igual ao valor final
    bool maiorQueTotal = portugues.toFloat(recebido, &conversionOkRecebido) >= portugues.toFloat(valor_final);
    qDebug() << conversionOkRecebido;
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


    query.prepare("INSERT INTO vendas2 (cliente, total, data_hora, forma_pagamento, "
                  "valor_recebido, troco, taxa, valor_final, desconto, id_cliente, esta_pago) "
                  "VALUES (:valor1, :valor2, :valor3, :valor4, :valor5, :valor6, :valor7, "
                  ":valor8, :valor9, :valor10, :valor11)");
    query.bindValue(":valor1", clienteGlobal);
    // precisa converter para notacao usa para inserir no banco de dados
    query.bindValue(":valor2", QString::number(portugues.toFloat(totalGlobal)));
    // inserir a data do dateedit
    query.bindValue(":valor3", dataIngles.toString("yyyy-MM-dd hh:mm:ss"));
    //
    query.bindValue(":valor4", forma_pagamento);
    // precisa converter para notacao usa para inserir no banco de dados
    query.bindValue(":valor5", QString::number(portugues.toFloat(recebido), 'f', 2));
    query.bindValue(":valor6", QString::number(portugues.toFloat(troco), 'f', 2));
    query.bindValue(":valor7", QString::number(portugues.toFloat(taxa), 'f', 2));
    query.bindValue(":valor8", QString::number(portugues.toFloat(valor_final), 'f', 2));
    query.bindValue(":valor9", QString::number(portugues.toFloat(desconto), 'f', 2));
    query.bindValue(":valor10", QString::number(idCliente));
    if(forma_pagamento == "Prazo"){
        query.bindValue(":valor11", "0");

    }else{
        query.bindValue(":valor11", "1");

    }

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
        query.bindValue(":valor2", QString::number(portugues.toFloat(rowdata[1].toString())));
        query.bindValue(":valor3", QString::number(portugues.toFloat(rowdata[3].toString()), 'f', 2));
        query.bindValue(":valor4", idVenda);
        if (query.exec()) {
            qDebug() << "Inserção prod_vendidos bem-sucedida!";
        } else {
            qDebug() << "Erro na inserção prod_vendidos: ";
        }
        query.prepare("UPDATE produtos SET quantidade = quantidade - :valor2 WHERE id = :valor1");
        query.bindValue(":valor1", rowdata[0]);
        // precisa converter para notacao usa para inserir no banco de dados
        query.bindValue(":valor2", QString::number(portugues.toFloat(rowdata[1].toString())));
        if (query.exec()) {
            qDebug() << "update quantidade bem-sucedida!";
        } else {
            qDebug() << "Erro na update quantidade: ";
        }

    }
    if(ui->CheckImprimirCNF->isChecked()){
        Vendas::imprimirReciboVenda(idVenda);
    }

    db.close();
    if (!waitDialog) {
        waitDialog = new WaitDialog(this);
    }
    waitDialog->setMessage("Aguardando resposta do servidor...");
    waitDialog->show();


    emit gerarEnviarNf();
    emit pagamentoConcluido(); // sinal para outras janelas atualizarem...
    verificarErroNf(this->nota.getCppNFe());
    imprimirDANFE(this->nota.getCppNFe());




    // fechar as janelas
    this->close();
}

void pagamentoVenda::imprimirDANFE(const CppNFe *cppnfe)
{
    QString caminhoReportNFe = QDir::currentPath() + "/reports/DANFE-NFe.xml";
    QString caminhoReportNFCe = QDir::currentPath() + "/reports/DANFE-NFCe.xml";

    CppDanfeQtRPT danfe(cppnfe, 0);
    //danfe.caminhoLogo(QDir::currentPath() + "/img/logo.png");
    if (cppnfe->notafiscal->retorno->protNFe->items->count() > 0)
    {
        if ((cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat() == 100) ||
            (cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat() == 150))
        {
            if (cppnfe->notafiscal->NFe->items->value(0)->infNFe->ide->get_mod() == ModeloDF::NFe)
                danfe.caminhoArquivo(caminhoReportNFe);
            else
                danfe.caminhoArquivo(caminhoReportNFCe);

            danfe.print();
        }
    } else
    {
        if (cppnfe->notafiscal->NFe->items->value(0)->infNFe->ide->get_mod() == ModeloDF::NFe)
            danfe.caminhoArquivo(caminhoReportNFe);
        else
            danfe.caminhoArquivo(caminhoReportNFCe);

        danfe.print();
    }
}
