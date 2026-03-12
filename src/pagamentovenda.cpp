#include "pagamentovenda.h"
#include "subclass/waitdialog.h"
#include <QTimer>
#include "configuracao.h"
#include <QSqlError>
#include "util/mailmanager.h"
#include <QDir>


pagamentoVenda::pagamentoVenda(QList<ProdutoVendidoDTO> listaProdutos, QString total, QString cliente,
                               QString data, qlonglong idCliente, QWidget *parent)
    : pagamento(total, cliente, data, parent)
{

    nfce = new NfceACBR(this);
    nfe = new NfeACBR(this, true, false);

    listaProds = listaProdutos;


    Config_service *confServ = new Config_service(this);
    configDTO = confServ->carregarTudo();

    this->idCliente = idCliente;
    //mostra as opçoes relacionadas a nf e

    if(configDTO.emitNfFiscal){
        ui->FrameNF->setVisible(true);
        ui->Ledit_NNF->setVisible(true);
        ui->Lbl_NNF->setVisible(true);
        QString nnf = QString::number(notaServ.getProximoNNF(configDTO.tpAmbFiscal,ModeloNota::NFCe));
        ui->Ledit_NNF->setText(nnf);
        ui->CBox_ModeloEmit->setVisible(true);
        ui->RadioBtn_EmitNfApenas->setVisible(true);
        ui->RadioBtn_EmitNfTodos->setVisible(true);

    }else{
        ui->FrameNF->setVisible(false);
        ui->Ledit_NNF->setVisible(false);
        ui->Lbl_NNF->setVisible(false);
        ui->CBox_ModeloEmit->setVisible(false);
        ui->RadioBtn_EmitNfApenas->setVisible(false);
        ui->RadioBtn_EmitNfTodos->setVisible(false);

    }

    //pega os dados do cliente necessários
    CLIENTE = cliServ.getClienteByID(idCliente);
    ui->Ledit_CpfCnpjCliente->setText(CLIENTE.cpf);

}

void pagamentoVenda::on_CBox_ModeloEmit_currentIndexChanged(int index)
{
    if(index == 0){
        ui->Ledit_NNF->setText(QString::number(notaServ.getProximoNNF(configDTO.tpAmbFiscal,
                                                                                ModeloNota::NFCe)));
        ui->RadioBtn_EmitNfApenas->setVisible(true);
        ui->RadioBtn_EmitNfTodos->setVisible(true);
        ui->Ledit_NNF->setVisible(true);
        ui->Lbl_NNF->setVisible(true);
    }else if(index == 1){
        ui->Ledit_NNF->setText(QString::number(notaServ.getProximoNNF(configDTO.tpAmbFiscal,
                                                                                ModeloNota::NFe)));
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

QString pagamentoVenda::enviarNfe(NfeACBR *nfe) {
    QString retorno = nfe->gerarEnviar();

    if (retorno.isEmpty())
        return "Erro: Nenhum retorno do ACBr";

    // Divide o retorno, tratando \r\n e linhas vazias
    QStringList linhas = retorno.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);

    // Variáveis de resposta
    bool dentroNFe = false;

    // Percorre o retorno linha a linha
    for (const QString &linha : linhas) {
        QString l = linha.trimmed();

        // Detecta início de seções
        if (l.startsWith('[')) {
            dentroNFe = l.startsWith("[NFe"); // começa a ler apenas dentro da seção NFe
            continue;
        }

        if (!dentroNFe)
            continue; // ignora dados de [Envio] ou outras seções

        QString key = l.section('=', 0, 0).trimmed().toLower();
        QString value = l.section('=', 1).trimmed();

        if (key == "cstat")
            cStat = value;
        else if (key == "xmotivo")
            xMotivo = value;
        else if (key == "msg")
            msg = value;
        else if (key == "nprot")
            nProt = value;
    }

    // Log opcional para depuração
    qDebug() << "Retorno NFe ACBr:"
             << "\ncStat:" << cStat
             << "\nxMotivo:" << xMotivo
             << "\nnProt:" << nProt;

    //  Avalia o resultado
    if (cStat == "100" || cStat == "150") {
        // Nota autorizada
        return QString("Nota Autorizada!\ncStat:%1\nmotivo:%2\nprotocolo:%3")
            .arg(cStat, xMotivo.isEmpty() ? msg : xMotivo, nProt);
    } else if (!cStat.isEmpty()) {
        // Nota rejeitada ou duplicada
        return QString("Nota Rejeitada\ncStat:%1\nmotivo:%2")
            .arg(cStat, xMotivo.isEmpty() ? msg : xMotivo);
    } else {
        // Retorno inesperado
        return QString("Erro: Resposta inesperada do ACBr\n-> %1")
            .arg(retorno.left(300));
    }
}

void pagamentoVenda::salvarNfeBD(NfeACBR *nfe){

    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. salvar nfce banco.";
    }
    QSqlQuery query;
    QDateTime dataIngles = portugues.toDateTime(dataGlobal, "dd-MM-yyyy hh:mm:ss");
    query.prepare("INSERT INTO notas_fiscais (cstat, nnf, serie, modelo, "
                  "tp_amb, xml_path, valor_total, atualizado_em, id_venda, cnpjemit, "
                  "chnfe, nprot, cuf, finalidade, saida, dhemi) "
                  "VALUES (:cstat, :nnf, :serie, :modelo, :tpamb, :xml_path, "
                  ":valortotal, :atualizado_em, :id_venda, :cnpjemit, :chnfe, "
                  ":nprot, :cuf, :finalidade, :saida, :dhemi)");
    query.bindValue(":cstat", cStat);
    query.bindValue(":nnf", QString::number(nfe->getNNF()));
    query.bindValue(":serie", QString::number(nfe->getSerie()));
    query.bindValue(":xml_path", nfe->getXmlPath());
    query.bindValue(":valortotal", QString::number(nfe->getVNF()));
    query.bindValue(":modelo", "55");
    query.bindValue(":tpamb", configDTO.tpAmbFiscal);
    query.bindValue(":atualizado_em", dataIngles.toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":id_venda", idVenda);
    query.bindValue(":cnpjemit", configDTO.cnpjEmpresa);
    query.bindValue(":chnfe", nfe->getChaveNf());
    query.bindValue(":nprot", nProt);
    query.bindValue(":cuf", configDTO.cUfFiscal);
    query.bindValue(":finalidade", "NORMAL");
    query.bindValue(":saida", "1");
    query.bindValue(":dhemi", nfe->getDhEmiConvertida());





    qDebug() << "idvenda: " << idVenda;
    if (query.exec()) {
        qDebug() << "Salvou nota no banco!";
    } else {
        qDebug() << "Erro ao salvar nota no banco:" << query.lastError().text();
        qDebug() << "Query:" << query.lastQuery();
    }


}

void pagamentoVenda::salvarNfceBD(NfceACBR *nfce){
    qDebug() << "inicio salvar bd nfce";
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. salvar nfce banco.";
    }
    QSqlQuery query;
    QDateTime dataIngles = portugues.toDateTime(dataGlobal, "dd-MM-yyyy hh:mm:ss");

    query.prepare("INSERT INTO notas_fiscais (cstat, nnf, serie, modelo, "
                  "tp_amb, xml_path, valor_total, atualizado_em, id_venda, cnpjemit,"
                  "chnfe, nprot, cuf, finalidade, saida, dhemi ) "
                  "VALUES (:cstat, :nnf, :serie, :modelo, :tpamb, :xml_path, "
                  ":valortotal, :atualizado_em, :id_venda, :cnpjemit, :chnfe, "
                  ":nprot, :cuf, :finalidade, :saida, :dhemi )");
    query.bindValue(":cstat", cStat);
    query.bindValue(":nnf", QString::number(nfce->getNNF()));
    query.bindValue(":serie", QString::number(nfce->getSerie()));
    query.bindValue(":xml_path", nfce->getXmlPath());
    query.bindValue(":valortotal", QString::number(nfce->getVNF()));
    query.bindValue(":modelo", "65");
    query.bindValue(":tpamb", configDTO.tpAmbFiscal);
    query.bindValue(":atualizado_em", dataIngles.toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":id_venda", idVenda);
    query.bindValue(":cnpjemit", configDTO.cnpjEmpresa);
    query.bindValue(":chnfe", nfce->getChaveNf());
    query.bindValue(":nprot", nProt);
    query.bindValue(":cuf", configDTO.cUfFiscal);
    query.bindValue(":finalidade", "NORMAL");
    query.bindValue(":saida", "1");
    query.bindValue(":dhemi", nfce->getDhEmiConvertida());

    qDebug() << "idvenda: " << idVenda;
    if (query.exec()) {
        qDebug() << "Salvou nota no banco!";
    } else {
        qDebug() << "Erro ao salvar nota no banco:" << query.lastError().text();
        qDebug() << "Query:" << query.lastQuery();
    }

}

void pagamentoVenda::terminarPagamento(){
    // inserir a venda
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

    QDateTime dataIngles = portugues.toDateTime(dataGlobal, "dd-MM-yyyy hh:mm:ss");

    VendasDTO  newVenda;
    newVenda.clienteNome = CLIENTE.nome;
    newVenda.dataHora = dataIngles.toString("yyyy-MM-dd hh:mm:ss");
    newVenda.desconto = portugues.toDouble(desconto);
    newVenda.formaPagamento = forma_pagamento;
    if(newVenda.formaPagamento == "Prazo"){
        newVenda.estaPago = false;
    }else{
        newVenda.estaPago = true;
    }
    newVenda.idCliente = idCliente;
    newVenda.taxa = portugues.toDouble(taxa);
    newVenda.total = portugues.toDouble(totalGlobal);
    newVenda.troco = portugues.toDouble(troco);
    newVenda.valorFinal = portugues.toDouble(valor_final);
    newVenda.valorRecebido = portugues.toDouble(recebido);

    ClienteDTO cli = CLIENTE;
    cli.cpf = ui->Ledit_CpfCnpjCliente->text().trimmed();

    auto result = vendaServ.inserirVendaRegraDeNegocio(newVenda, listaProds);
    if(!result.ok){
        QMessageBox::warning(this, "Erro", result.msg);
        return;
    }else{
        newVenda.id = result.idVendaInserida;
    }

    //se emitir NFE

    // if(ui->CBox_ModeloEmit->currentIndex() == 1 && (nomeCli.isEmpty() ||
    //     emailCli.isEmpty() ||
    //     enderecoCli.isEmpty() || cpfCli.isEmpty() || numeroCli.isEmpty() ||
    //     bairroCli.isEmpty() || xMunCli.isEmpty() || cMunCli.isEmpty() ||
    //     ufCli.isEmpty() || cepCli.isEmpty())){
    //     QMessageBox::warning(this,"Erro", "Cliente com dados incompletos para emitir NF-E");
    //     return;
    // }

    if(ui->CheckImprimirCNF->isChecked()){
        Vendas::imprimirReciboVenda(idVenda.toLongLong());
    }

    bool okEmitir = true;

    if(configDTO.emitNfFiscal && okEmitir){ // se a config estiver ativada para emitir

        if(ui->CBox_ModeloEmit->currentIndex() == 0){ // se emitir NFCE

            qlonglong nnf = ui->Ledit_NNF->text().toLongLong();
            auto result1 = fiscalServ.enviarNfcePadrao(newVenda, listaProds, nnf, cli, emitTodosNf, false);

            if(!result1.ok && result1.erro == FiscalEmitterErro::NCMInvalido){
                QMessageBox::StandardButton resposta;
                resposta = QMessageBox::question(this,
                                                 "Atenção", result1.msg + "\nDeseja continuar mesmo assim?",
                                                 QMessageBox::Yes | QMessageBox::No);
                if(resposta == QMessageBox::No){
                    return;
                }
                if(resposta == QMessageBox::Yes){
                    result1 = fiscalServ.enviarNfcePadrao(newVenda, listaProds, nnf, cli,
                                                         emitTodosNf, true);;
                }
            }

            if (!waitDialog) {
                waitDialog = new WaitDialog(this);
            }
            waitDialog->setMessage("Aguardando resposta do servidor...");
            waitDialog->show();
            waitDialog->allowClose();
            if(!result1.ok){
                waitDialog->setMessage(result1.msg);
            }else{
                waitDialog->setMessage(result1.msg);
                QTimer::singleShot(1500, waitDialog, &WaitDialog::close); //fecha depois de 2 seg
            }

        }else if(ui->CBox_ModeloEmit->currentIndex() == 1){


            // if (!waitDialog) {
            //     waitDialog = new WaitDialog(this);
            // }

            // nfe->setNNF(ui->Ledit_NNF->text().toInt());
            // nfe->setCliente(ehPfCli, cpfCli, nomeCli, indIeCLi, emailCli, enderecoCli,
            //                 numeroCli, bairroCli, cMunCli, xMunCli, ufCli, cepCli, ieCli);
            // nfe->setProdutosVendidos(rowDataList, emitTodosNf);
            // nfe->setPagamentoValores(forma_pagamento,portugues.toFloat(desconto),portugues.toFloat(recebido), portugues.toFloat(troco), taxa.toFloat());

            // waitDialog->setMessage("Aguardando resposta do servidor...");
            // waitDialog->show();
            // waitDialog->allowClose();
            // waitDialog->setMessage(enviarNfe(nfe));
            // if(cStat == "100" || cStat == "150"){
            //     salvarNfeBD(nfe); //salva nfe no banco
            //     if(nfe->getTpAmb() == "1"){
            //         enviarEmailNFe(nomeCli, emailCli, nfe->getXmlPath(),
            //                        nfe->getPdfDanfe());
            //     }
            //     QTimer::singleShot(1500, waitDialog, &WaitDialog::close); //fecha depois de 2 seg

            // }
        }else if(ui->CBox_ModeloEmit->currentIndex() == 2){
        }

    }
    emit pagamentoConcluido();
    // fechar as janelas
    this->close();

}

void pagamentoVenda::enviarEmailNFe(QString nomeCliente, QString emailCliente,
                                    QString xmlPath, std::string pdfDanfe){

    try {

        QDateTime data = portugues.toDateTime(dataGlobal, "dd-MM-yyyy hh:mm:ss");

        auto mail = MailManager::instance().mail();
        QByteArray pdfBytes = QByteArray::fromBase64(
            QByteArray::fromStdString(pdfDanfe)
            );
        QString pdfPath =
            QDir::tempPath() + "/DANFE_" +
            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") +
            ".pdf";

        QFile pdfFile(pdfPath);
        if (!pdfFile.open(QIODevice::WriteOnly)) {
            qDebug() << "Erro ao criar PDF DANFE";
            return;
        }
        pdfFile.write(pdfBytes);
        pdfFile.close();

        QString corpo;
        QString nomeEmpresa = configDTO.nomeEmpresa;
        QString dataFormatada = portugues.toString(
            data,
            "dddd, dd 'de' MMMM 'de' yyyy 'às' HH:mm"
            );

        corpo = "Olá " + nomeCliente + "\n\n"
                                       "Agradecemos por comprar da " + nomeEmpresa + "!\n"
                                       "em anexo, você encontrará os arquivos referentes à "
                                "Nota Fiscal da compra de " +
                                       dataFormatada + ".\n\n"
                                       "Cordialmente,\n\n" +
                                       nomeEmpresa;
        mail->Limpar();
        mail->LimparAnexos();
        mail->AddCorpoAlternativo(corpo.toStdString());
        mail->SetAssunto("Nota Fiscal Eletrônica de " + configDTO.nomeEmpresa.toStdString());
        mail->AddDestinatario(emailCliente.toStdString());
        mail->AddAnexo(xmlPath.toStdString(), "XML NFe", 0);
        mail->AddAnexo(pdfPath.toStdString(), "DANFE (PDF)", 0);

        mail->Enviar();
        qDebug() << "email enviado NFE";
    }
    catch (const std::exception& e) {
        qDebug() << "email não enviado NFE";
    }
}
