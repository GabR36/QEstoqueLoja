#include "pagamentovenda.h"
#include "subclass/waitdialog.h"
#include <QTimer>
#include "configuracao.h"
#include <QSqlError>


pagamentoVenda::pagamentoVenda(QList<ProdutoVendidoDTO> listaProdutos, QString total, QString cliente,
                               QString data, qlonglong idCliente, QWidget *parent)
    : pagamento(total, cliente, data, parent)
{

    // nfce = new NfceACBR(this);
    // nfe = new NfeACBR(this, true, false);

    listaProds = listaProdutos;

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
                    auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                    if(!r1.ok){
                        QMessageBox::warning(this,"Erro", r1.msg);
                    }
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
                auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                if(!r1.ok){
                    QMessageBox::warning(this,"Erro", r1.msg);
                }
            }else{
                waitDialog->setMessage(result1.msg);
                QTimer::singleShot(1500, waitDialog, &WaitDialog::close); //fecha depois de 2 seg
            }

        }else if(ui->CBox_ModeloEmit->currentIndex() == 1){ // emitir nfe


            qlonglong nnf = ui->Ledit_NNF->text().toLongLong();
            auto result1 = fiscalServ.enviarNFePadrao(newVenda, listaProds, nnf, cli, emitTodosNf, false);

            if(!result1.ok && result1.erro == FiscalEmitterErro::NCMInvalido){
                QMessageBox::StandardButton resposta;
                resposta = QMessageBox::question(this,
                                                 "Atenção", result1.msg + "\nDeseja continuar mesmo assim?",
                                                 QMessageBox::Yes | QMessageBox::No);
                if(resposta == QMessageBox::No){
                    auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                    if(!r1.ok){
                        QMessageBox::warning(this,"Erro", r1.msg);
                    }
                    return;
                }
                if(resposta == QMessageBox::Yes){
                    result1 = fiscalServ.enviarNFePadrao(newVenda, listaProds, nnf, cli,
                                                          emitTodosNf, true);
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
                auto r1 = vendaServ.deletarVendaRegraNegocio(newVenda.id, false);
                if(!r1.ok){
                    QMessageBox::warning(this,"Erro", r1.msg);
                }
                return;
            }else{
                waitDialog->setMessage(result1.msg);
                QTimer::singleShot(1500, waitDialog, &WaitDialog::close); //fecha depois de 2 seg
            }

        }else if(ui->CBox_ModeloEmit->currentIndex() == 2){ // nao emitir nf
        }

    }
    emit pagamentoConcluido();
    // fechar as janelas
    this->close();

}

