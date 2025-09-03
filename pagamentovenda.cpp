#include "pagamentovenda.h"
#include "subclass/waitdialog.h"
#include <QTimer>
#include "configuracao.h"
#include <QSqlError>
pagamentoVenda::pagamentoVenda(QList<QList<QVariant>> listaProdutos, QString total, QString cliente, QString data, int idCliente, QWidget *parent)
    : pagamento(total, cliente, data, parent)
{

    rowDataList = listaProdutos;
    fiscalValues = Configuracao::get_All_Fiscal_Values();
    empresaValues = Configuracao::get_All_Empresa_Values();
    this->idCliente = idCliente;
    //mostra as opçoes relacionadas a nf e
    if(fiscalValues.value("emit_nf") == "1"){
        ui->FrameNF->setVisible(true);
        ui->Ledit_NNF->setVisible(true);
        ui->Lbl_NNF->setVisible(true);
        ui->Ledit_NNF->setText(QString::number(notaNFCe.getProximoNNF()));

    }else{
        ui->FrameNF->setVisible(false);
        ui->Ledit_NNF->setVisible(false);
        ui->Lbl_NNF->setVisible(false);

    }

    //pega os dados do cliente necessários
    if(!db.open()){
        qDebug() << "erro bd cliente";
    }else{
        QSqlQuery query;
        query.prepare("SELECT cpf, eh_pf, nome, email, telefone, endereco, "
                      "numero_end, bairro, xMun, cMun, uf, cep, indIEDest, ie"
                      "  FROM clientes WHERE id = :idcliente");
        query.bindValue(":idcliente", idCliente);
        if (!query.exec()) {
            qDebug() << "cliente nao encontrado";
        }
        while(query.next()) {
            cpfCli = query.value(0).toString();
            ehPfCli = query.value(1).toBool();
            nomeCli = query.value(2).toString();
            emailCli = query.value(3).toString();
            telefoneCli = query.value(4).toString();
            enderecoCli = query.value(5).toString();
            numeroCli = query.value(6).toString();
            bairroCli = query.value(7).toString();
            xMunCli = query.value(8).toString();
            cMunCli = query.value(9).toString();
            ufCli = query.value(10).toString();
            cepCli = query.value(11).toString();
            indIeCLi = query.value(12).toInt();
            ieCli = query.value(13).toString();
        }
    }
    ui->Ledit_CpfCnpjCliente->setText(cpfCli);

}

void pagamentoVenda::on_CBox_ModeloEmit_currentIndexChanged(int index)
{
    if(index == 0){
        ui->Ledit_NNF->setText(QString::number(notaNFCe.getProximoNNF()));
        ui->RadioBtn_EmitNfApenas->setVisible(true);
        ui->RadioBtn_EmitNfTodos->setVisible(true);
        ui->Ledit_NNF->setVisible(true);
        ui->Lbl_NNF->setVisible(true);
    }else if(index == 1){
        ui->Ledit_NNF->setText(QString::number(notaNFe.getProximoNNF()));
        ui->RadioBtn_EmitNfApenas->setVisible(true);
        ui->RadioBtn_EmitNfTodos->setVisible(true);
        ui->Ledit_NNF->setVisible(true);
        ui->Lbl_NNF->setVisible(true);
    }
    else if(index == 2){
        ui->RadioBtn_EmitNfApenas->setVisible(false);
        ui->RadioBtn_EmitNfTodos->setVisible(false);
        ui->Ledit_NNF->setVisible(false);
        ui->Lbl_NNF->setVisible(false);
    }else{

    }
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
void pagamentoVenda::onRetLote(const QString &lote)
{
    if (waitDialog) {
        waitDialog->allowClose();
        waitDialog->setMessage(lote);
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
        if(ui->CBox_ModeloEmit->currentIndex() == 0){
            query.bindValue(":nnf", QString::number(notaNFCe.getNNF()));
            query.bindValue(":serie", QString::number(notaNFCe.getSerie()));
            query.bindValue(":xml_path", notaNFCe.getXmlPath());
            query.bindValue(":valortotal", QString::number(notaNFCe.getVNF(),'f', 2));
            query.bindValue(":modelo", "65");
        }else if(ui->CBox_ModeloEmit->currentIndex() == 1){
            query.bindValue(":nnf", QString::number(notaNFe.getNNF()));
            query.bindValue(":serie", QString::number(notaNFe.getSerie()));
            query.bindValue(":xml_path", notaNFe.getXmlPath());
            query.bindValue(":valortotal", QString::number(notaNFe.getVNF(),'f', 2));
            query.bindValue(":modelo", "55");
        }


        query.bindValue(":tpamb", fiscalValues.value("tp_amb"));
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


    //se emitir NFE

    if(ui->CBox_ModeloEmit->currentIndex() == 1 && (nomeCli.isEmpty() ||
        emailCli.isEmpty() ||
        enderecoCli.isEmpty() || cpfCli.isEmpty() || numeroCli.isEmpty() ||
        bairroCli.isEmpty() || xMunCli.isEmpty() || cMunCli.isEmpty() ||
        ufCli.isEmpty() || cepCli.isEmpty())){
        QMessageBox::warning(this,"Erro", "Cliente com dados incompletos para emitir NF-E");
        return;
    }
    bool okEmitir = true;
    if(ui->RadioBtn_EmitNfTodos->isChecked()){
        emitTodosNf = true;
    }else{
        emitTodosNf = false;
    }
    if(!emitTodosNf && !existeProdutosComNF(rowDataList) ){
        okEmitir = false;
        QMessageBox::warning(this,"Atenção", "Não foram encontrados produtos NF na venda, "
                                              "portanto não será emitido nota");
    }else{
        okEmitir = true;
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
        if (ehPfCli == true && cpf.length() != 11){
            QMessageBox::warning(this, "Erro", "Por favor, insira um cpf com 11 digitos.");
            return;
        }
        if (ehPfCli == false && cpf.length() != 14){
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

    if((existeItensComNcmVazio(rowDataList, emitTodosNf)) && (fiscalValues.value("emit_nf") == "1" && okEmitir)
        && (ui->CBox_ModeloEmit->currentIndex() != 2)){
        QMessageBox::StandardButton resposta;
        resposta = QMessageBox::question(this,
                                         "Atenção", "Existem produtos com NCM vazio ou igual a '0000000', Deseja continuar mesmo assim?",
                                         QMessageBox::Yes | QMessageBox::No);
        if(resposta == QMessageBox::No){
            return;
        }
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
    if(fiscalValues.value("emit_nf") == "1" && okEmitir){ // se a config estiver ativada para emitir

        if(ui->CBox_ModeloEmit->currentIndex() == 0){

            if (!waitDialog) {
                waitDialog = new WaitDialog(this);
            }
            waitDialog->setMessage("Aguardando resposta do servidor...");
            waitDialog->show();

            connect(this, &pagamentoVenda::gerarEnviarNf, &notaNFCe, &NfceVenda::onReqGerarEnviar);
            connect(&notaNFCe, &NfceVenda::retWSChange, this, &pagamentoVenda::onRetWSChange);
            connect(&notaNFCe, &NfceVenda::errorOccurred, this, &pagamentoVenda::onErrorOccurred);
            connect(&notaNFCe, &NfceVenda::retStatusServico, this, &pagamentoVenda::onRetStatusServico);
            connect(&notaNFCe, &NfceVenda::retLote, this, &pagamentoVenda::onRetLote);

            notaNFCe.setNNF(ui->Ledit_NNF->text().toInt());
            notaNFCe.setCliente(cpf, ehPfCli);
            notaNFCe.setProdutosVendidos(rowDataList, emitTodosNf);
            notaNFCe.setPagamentoValores(forma_pagamento,portugues.toFloat(desconto),portugues.toFloat(recebido), portugues.toFloat(troco), taxa.toFloat());
            emit gerarEnviarNf();

            verificarErroNf(this->notaNFCe.getCppNFe());
        }else if(ui->CBox_ModeloEmit->currentIndex() == 1){
            if (!waitDialog) {
                waitDialog = new WaitDialog(this);
            }
            waitDialog->setMessage("Aguardando resposta do servidor...");
            waitDialog->show();

            connect(this, &pagamentoVenda::gerarEnviarNf, &notaNFe, &NFeVenda::onReqGerarEnviar);
            connect(&notaNFe, &NFeVenda::retWSChange, this, &pagamentoVenda::onRetWSChange);
            connect(&notaNFe, &NFeVenda::errorOccurred, this, &pagamentoVenda::onErrorOccurred);
            connect(&notaNFe, &NFeVenda::retStatusServico, this, &pagamentoVenda::onRetStatusServico);
            connect(&notaNFe, &NFeVenda::retLote, this, &pagamentoVenda::onRetLote);

            notaNFe.setNNF(ui->Ledit_NNF->text().toInt());
            notaNFe.setCliente(ehPfCli, cpfCli, nomeCli, indIeCLi, emailCli, enderecoCli,
            numeroCli, bairroCli, cMunCli, xMunCli, ufCli, cepCli, ieCli);
            notaNFe.setProdutosVendidos(rowDataList, emitTodosNf);
            notaNFe.setPagamentoValores(forma_pagamento,portugues.toFloat(desconto),portugues.toFloat(recebido), portugues.toFloat(troco), taxa.toFloat());
            emit gerarEnviarNf();

            verificarErroNf(this->notaNFe.getCppNFe());
        }else if(ui->CBox_ModeloEmit->currentIndex() == 2){
        }

    }
    emit pagamentoConcluido();
    // fechar as janelas
    this->close();

}
bool pagamentoVenda::existeItensComNcmVazio(QList<QList<QVariant>> listaProdutos, bool somenteNf) {
    QSqlQuery query;
    db.open();

    for (const QList<QVariant> &produto : listaProdutos) {
        if (produto.isEmpty())
            continue;

        QVariant idProduto = produto.at(0);

        QString sql = "SELECT ncm, nf FROM produtos WHERE id = :id";
        query.prepare(sql);
        query.bindValue(":id", idProduto);

        if (!query.exec()) {
            qWarning() << "Erro ao consultar produto ID" << idProduto << ":" << query.lastError().text();
            continue;
        }

        if (query.next()) {
            QString ncm = query.value("ncm").toString().trimmed();
            bool nf = query.value("nf").toBool();

            if (somenteNf && !nf) {
                // pula este produto, pois nf não é true
                continue;
            }

            if (ncm.isEmpty() || ncm == "00000000") {
                qDebug() << "Produto ID" << idProduto << "com NCM inválido:" << ncm;
                return true;
            }
        } else {
            qWarning() << "Produto ID" << idProduto << "não encontrado no banco de dados.";
        }
    }

    return false;
}
bool pagamentoVenda::existeProdutosComNF(QList<QList<QVariant>> listaProdutos){
    QSqlQuery query;
    db.open();
    int quantidadeProdutosNF = 0;

    for (const QList<QVariant> &produto : listaProdutos) {
        if (produto.isEmpty())
            continue;
        QVariant idProduto = produto.at(0);

        QString sql = "SELECT nf FROM produtos WHERE id = :id";
        query.prepare(sql);
        query.bindValue(":id", idProduto);

        if (!query.exec()) {
            qWarning() << "Erro ao consultar produto ID" << idProduto << ":" << query.lastError().text();
            continue;
        }
        if (query.next()) {
            bool nf = query.value("nf").toBool();

            if (nf) {
                quantidadeProdutosNF++;
            }
        } else {
            qWarning() << "Produto ID" << idProduto << "não encontrado no banco de dados.";
        }
    }
    if(quantidadeProdutosNF > 0){
        return true;
    }else{
        return false;
    }
}
