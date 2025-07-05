#include "pagamentovenda.h"
#include "subclass/waitdialog.h"
#include <QTimer>
#include "configuracao.h"
pagamentoVenda::pagamentoVenda(QList<QList<QVariant>> listaProdutos, QString total, QString cliente, QString data, int idCliente, QWidget *parent)
    : pagamento(total, cliente, data, parent)
{

    rowDataList = listaProdutos;
    fiscalValues = Configuracao::get_All_Fiscal_Values();
    empresaValues = Configuracao::get_All_Empresa_Values();
    this->idCliente = idCliente;


        if(!db.open()){
            qDebug() << "erro bd cliente";
        }else{
            QSqlQuery query;
            query.prepare("SELECT cpf, eh_pf FROM clientes WHERE id = :idcliente");
            query.bindValue(":idcliente", idCliente);
            if (!query.exec()) {
                qDebug() << "cliente nao encontrado";
            }
            while(query.next()) {
                QString cpf = query.value(0).toString();
                bool ehPf = query.value(1).toBool();
                ui->Ledit_CpfCnpjCliente->setText(cpf);
                ehPfCliente = ehPf;
            }
        }

        connect(this, &pagamentoVenda::gerarEnviarNf, &nota, &NfceVenda::onReqGerarEnviar);
        connect(&nota, &NfceVenda::retWSChange, this, &pagamentoVenda::onRetWSChange);
        connect(&nota, &NfceVenda::errorOccurred, this, &pagamentoVenda::onErrorOccurred);
        connect(&nota, &NfceVenda::retStatusServico, this, &pagamentoVenda::onRetStatusServico);



}
void pagamentoVenda::onErrorOccurred(const QString &error){
    if (waitDialog) {
        waitDialog->allowClose();
        waitDialog->setMessageErro(error);
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
        QString cStatMessage;
        if ((cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat() == 100) ||
            (cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat() == 150))
        {

            cStatMessage = QString::number(cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat());
            waitDialog->setMessage("Sucesso!\n Status:" + cStatMessage);
            waitDialog->allowClose();

            QTimer::singleShot(2000, waitDialog, &WaitDialog::close); //fecha depois de 2 segundos
            //imprimirDANFE(this->nota.getCppNFe());
        }else{
            QString cStatMessage = QString::number(cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat());
            QString msg = "ERRO:\n" + cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_xMotivo() +
                          "\n" +"cstat: " + cStatMessage ;
            qDebug() << "Erro nf:" << msg;
            waitDialog->setMessage(msg);
            waitDialog->allowClose();
            //QTimer::singleShot(2000, waitDialog, &QDialog::close);
        }
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. botao sucesso nota.";
        }
        QSqlQuery query;
        QDateTime dataIngles = portugues.toDateTime(dataGlobal, "dd-MM-yyyy hh:mm:ss");

        query.prepare("INSERT INTO notas_fiscais (cstat, nnf, serie, modelo, "
                      "tp_amb, xml_path, valor_total, atualizado_em, id_venda) "
                      "VALUES (:cstat, :nnf, :serie, :modelo, :tpamb, :xml_path, :valortotal, :atualizado_em, :id_venda)");
        query.bindValue(":cstat", cStatMessage);
        query.bindValue(":nnf", QString::number(nota.getNNF()));
        query.bindValue(":serie", QString::number(nota.getSerie()));
        query.bindValue(":modelo", "65");
        query.bindValue(":tpamb", fiscalValues.value("tp_amb"));
        query.bindValue(":xml_path", nota.getXmlPath());
        query.bindValue(":valortotal", QString::number(nota.getVNF(),'f', 2));
        query.bindValue(":atualizado_em", dataIngles.toString("yyyy-MM-dd hh:mm:ss"));
        query.bindValue(":id_venda", idVenda);
        qDebug() << "idvenda: " << idVenda;
        if (query.exec()) {
            qDebug() << "Salvou nota no banco!";
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
    if(ui->RadioBtn_EmitNfTodos->isChecked()){
        emitTodosNf = true;
    }else{
        emitTodosNf = false;
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
    QString cpf = ui->Ledit_CpfCnpjCliente->text().trimmed();
    if(cpf != ""){
        if (ehPfCliente == true && cpf.length() != 11){
            QMessageBox::warning(this, "Erro", "Por favor, insira um cpf com 11 digitos.");
            return;
        }
        if (ehPfCliente == false && cpf.length() != 14){
            QMessageBox::warning(this, "Erro", "Por favor, insira um cnpj com 14 digitos.");
            return;
        }
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
    if(fiscalValues.value("emit_nf") == "1"){ // se a config estiver ativada para emitir
        if (!waitDialog) {
            waitDialog = new WaitDialog(this);
        }



        waitDialog->setMessage("Aguardando resposta do servidor...");
        waitDialog->show();
        nota.setCliente(cpf, ehPfCliente);
        nota.setProdutosVendidos(rowDataList, emitTodosNf);
        nota.setPagamentoValores(forma_pagamento,portugues.toFloat(desconto),portugues.toFloat(recebido), portugues.toFloat(troco), taxa.toFloat());
        emit gerarEnviarNf();
        emit pagamentoConcluido(); // sinal para outras janelas atualizarem...

        verificarErroNf(this->nota.getCppNFe());
    }
    emit pagamentoConcluido(); // sinal para outras janelas atualizarem...

    // fechar as janelas
    this->close();

}
