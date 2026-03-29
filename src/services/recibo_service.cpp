#include "recibo_service.h"
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include "../infra/databaseconnection_service.h"
#include "config_service.h"


Recibo_service::Recibo_service(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
    portugues = QLocale(QLocale::Portuguese, QLocale::Brazil);
}


void Recibo_service::imprimirReciboVenda(qlonglong idvenda){

    QPrinter printer;

    printer.setPageSize(QPageSize(QSizeF(80, 2000), QPageSize::Millimeter));// Tamanho do papel
    // printer.pageLayout().setPageSize(customPageSize);
    printer.setFullPage(true); // Utilizar toda a página        QPrintDialog dialog(&printer, this);

    QPrintDialog dialog(&printer);
    if(dialog.exec() == QDialog::Rejected) return;

    QPainter painter;
    painter.begin(&printer);
    QFont font = painter.font();
    font.setPointSize(8);
    font.setBold(true);
    painter.setFont(font);


    ConfigDTO configs = confServ.carregarTudo();

    VendasDTO venda = vendaServ.getVenda(idvenda);

    QDateTime dataVenda = QDateTime::fromString(
        venda.dataHora,
        "yyyy-MM-dd hh:mm:ss"
        );


    int yPos = 30; // Posição inicial para começar a desenhar o texto
    const int margemEsquerda = 20;
    int xPos = margemEsquerda;
    const int xPosPrm = margemEsquerda;
    const int yPosPrm = 10; // Posição inicial para começar a desenhar o texto
    //  painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(85, 10, "Cupom Compra Venda");
    yPos += 20; // Avança a posição y
    painter.drawText(xPos, yPos, configs.nomeEmpresa);
    yPos += 20;
    painter.drawText(xPos, yPos, configs.enderecoEmpresa);
    yPos += 20;
    painter.drawText(xPos, yPos, configs.cnpjEmpresa);
    yPos += 20;
    painter.drawText(xPos, yPos, configs.telefoneEmpresa);
    yPos += 20;
    painter.drawText(xPos, yPos, "Data/Hora: " + portugues.toString(dataVenda, "dd/MM/yyyy HH:mm"));
    yPos += 20;
    painter.drawText(xPos, yPos, "Cliente: " + venda.clienteNome);
    yPos += 30;
    painter.drawText(xPos, yPos, "Quant:");
    int xPosProds = 65;
    xPos = xPosProds;
    painter.drawText(xPos, yPos, "Produtos vendidos:");
    int xPosValor = 210;
    xPos = xPosValor;
    painter.drawText(xPos, yPos, "Valor(R$):");
    yPos += 20;

    font.setPointSize(8);
    font.setBold(false);
    painter.setFont(font);
    //painter.setFont(QFont("Arial", 10));
    int lineHeight = 25; // Altura da linha
    int pageWidth = printer.pageLayout().paintRectPixels(printer.resolution()).width();

    auto produtos = prodVendaServ.getProdutosVendidos(idvenda);
    //QStringList descricoes = getDescricoesProdutos(produtos);
    //int index = 0;  // Índice para acessar as descrições

    for (int i = 0; i < produtos.size(); i++) {
        QString descricaoProduto = produtos[i].descricao;
        double quantidadProduto = produtos[i].quantidade;
        double valorProduto = produtos[i].precoVendido;

        //tudo isso para formatar o valor maior q 1k para br
        double totalprods = valorProduto * quantidadProduto;
        QString totalFormatado = portugues.toString(totalprods, 'f', 2);



        QTextOption textOption;

        QRect rectQuantProd(xPosPrm, yPos, xPosProds - xPosPrm, lineHeight);
        painter.drawText(rectQuantProd, QString::number(quantidadProduto), textOption);

        QRect rectDesc(xPosProds, yPos, xPosValor - xPosProds - 30, lineHeight);
        textOption.setWrapMode(QTextOption::WordWrap);
        painter.drawText(rectDesc, descricaoProduto, textOption);

        QRect rectValor(xPosValor, yPos, pageWidth - xPosValor, lineHeight);
        painter.drawText(rectValor, totalFormatado, textOption);
        yPos += lineHeight;
    }
    int posx = xPosPrm;
    yPos += 5;
    for(int i=0; i < pageWidth; i++){
        posx += 3;
        painter.drawText(posx,yPos, "=");
    };
    // font.setBold(true);
    // painter.setFont(font);
    yPos += 20;
    //    painter.drawText(Qt::AlignCenter,yPos, "Pagamento");
    xPos = 95;
    painter.drawText(xPos,yPos, "Desconto(R$): " + portugues.toString(venda.desconto,'f',2));
    yPos += 20;
    painter.drawText(xPos,yPos, "Forma Pagamento: " + venda.formaPagamento);
    yPos += 20;
    painter.drawText(xPos,yPos, "Valor Total Produtos(R$): " + portugues.toString(venda.total,'f',2));
    yPos += 20;


    if(venda.formaPagamento == "Dinheiro" ){
        painter.drawText(xPos,yPos, "Valor Final(R$): " + portugues.toString(venda.valorFinal,'f',2));
        yPos += 20;
        painter.drawText(xPos, yPos, "Valor Recebido(R$):" + portugues.toString(venda.valorRecebido,'f',2));
        yPos += 20;
        painter.drawText(xPos,yPos, "Troco(R$):" + portugues.toString(venda.troco,'f',2));
    }else if(venda.formaPagamento == "Não Sei"){
        painter.drawText(xPos,yPos, "Valor Final(R$): " + portugues.toString(venda.valorFinal,'f',2));
        yPos += 20;
    }else if(venda.formaPagamento == "Crédito"){
        painter.drawText(xPos, yPos, "Taxa(%):" + portugues.toString(venda.taxa,'f',2));
        yPos += 20;
        painter.drawText(xPos, yPos, "Valor Final(R$):" + portugues.toString(venda.valorFinal, 'f', 2 ));

    }else if(venda.formaPagamento == "Débito"){
        painter.drawText(xPos, yPos, "Taxa(%):" + portugues.toString(venda.taxa,'f',2));
        yPos += 20;
        painter.drawText(xPos, yPos, "Valor Final(R$):" + portugues.toString(venda.valorFinal, 'f', 2 ));

    }else if(venda.formaPagamento == "Pix"){
        painter.drawText(xPos,yPos, "Valor Final(R$): " + portugues.toString(venda.valorFinal,'f',2));
        yPos += 20;
    }else if(venda.formaPagamento == "Prazo"){
        painter.drawText(xPos,yPos, "Valor Final(R$): " + portugues.toString(venda.valorFinal,'f',2));
        yPos += 20;
        posx = 0;
        for(int i=0; i < pageWidth; i++){
            painter.drawText(posx, yPos, "=");
            posx += 6;
        }

        QList<EntradaVendaDTO> listaEntradas;
        listaEntradas = entradaServ.getEntradasFromVenda(idvenda);

        int rowEntrada = 1;
        double devendoEntrada = venda.valorFinal;
        int xPosParcela = xPosValor - 70;

        for(int i = 0; i < listaEntradas.size(); i++){
            double totalEntrada = listaEntradas[i].total;
            QString data_horaEntrada = listaEntradas[i].dataHora;
            QString forma_pagamentoEntrada = listaEntradas[i].formaPagamento;
            double valor_recebidoEntrada = listaEntradas[i].valorRecebido;
            double trocoEntrada = listaEntradas[i].troco;
            double taxaEntrada = listaEntradas[i].taxa;
            double valorFinalEntrada = listaEntradas[i].valorFinal;

            QDate dataEntrada = portugues.toDate(data_horaEntrada, "dd/MM/yyyy hh:mm");
            // QString descontoEntrada = query.value("desconto").toString();
            xPos = 60;
            yPos += 30;
            for(int i=0; i < pageWidth; i++){
                posx += 3;
                painter.drawText(posx,yPos, "=");
            };
            font.setBold(true);
            font.setUnderline(true);
            painter.setFont(font);
            painter.drawText(xPos, yPos, "Parcela: " + QString::number(rowEntrada));
            yPos += 20;
            xPos=xPosPrm;
            font.setUnderline(false);
            font.setBold(false);
            painter.setFont(font);

            if(forma_pagamentoEntrada == "Dinheiro" ){
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(xPos, yPos, "Descontado(R$): " + portugues.toString(totalEntrada,'f',2));
                font.setBold(false);
                painter.setFont(font);
                xPos = xPosParcela;
                painter.drawText(xPos, yPos, "Data: " + dataEntrada.toString());
                yPos += 20;
                xPos = xPosPrm;
                painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);
                xPos = xPosParcela;
                painter.drawText(xPos, yPos, "Valor Rec.(R$): " + portugues.toString(valor_recebidoEntrada,'f',2));
                yPos += 20;
                painter.drawText(xPos,yPos, "Troco(R$): " + portugues.toString(trocoEntrada,'f',2));
            }else if(forma_pagamentoEntrada == "Não Sei"){
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(xPos, yPos, "Descontado(R$): " + portugues.toString(totalEntrada,'f',2));
                font.setBold(false);
                painter.setFont(font);
                xPos = xPosParcela;
                painter.drawText(xPos, yPos, "Data: " + dataEntrada.toString());
                yPos += 20;
                painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);
                yPos += 20;
            }else if(forma_pagamentoEntrada == "Crédito"){
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(xPos, yPos, "Descontado(R$): " + portugues.toString(totalEntrada,'f',2));
                font.setBold(false);
                painter.setFont(font);
                xPos = xPosParcela;
                painter.drawText(xPos, yPos, "Data: " + dataEntrada.toString());
                yPos += 20;
                xPos = xPosPrm;
                painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);
                xPos = xPosParcela;
                painter.drawText(xPos, yPos, "Taxa(%): " + portugues.toString(taxaEntrada, 'f',2));
                yPos += 20;
                painter.drawText(xPos, yPos, "Valor Final(R$): " + portugues.toString(valorFinalEntrada, 'f', 2 ));

            }else if(forma_pagamentoEntrada == "Débito"){
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(xPos, yPos, "Descontado(R$): " + portugues.toString(totalEntrada,'f',2));
                font.setBold(false);
                painter.setFont(font);
                xPos = xPosParcela;
                painter.drawText(xPos, yPos, "Data: " + dataEntrada.toString());
                yPos += 20;
                xPos = xPosPrm;
                painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);
                xPos = xPosParcela;
                painter.drawText(xPos, yPos, "Taxa(%): " + portugues.toString(taxaEntrada,'f',2));
                yPos += 20;
                painter.drawText(xPos, yPos, "Valor Final(R$): " + portugues.toString(valorFinalEntrada, 'f', 2 ));

            }else if(forma_pagamentoEntrada == "Pix"){
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(xPos, yPos, "Descontado(R$): " + portugues.toString(totalEntrada,'f',2));
                font.setBold(false);
                painter.setFont(font);
                xPos = xPosParcela;
                painter.drawText(xPos, yPos, "Data: " + dataEntrada.toString());
                yPos += 20;
                xPos = xPosPrm;
                painter.drawText(xPos, yPos, "Forma Pag: " + forma_pagamentoEntrada);

            }else{
                qDebug() << "forma de pagamentro entrada nao encontrada";
            }
            for(int i=0; i < pageWidth; i++){
                posx += 3;
                painter.drawText(posx,yPos, "=");
            };


            devendoEntrada -= totalEntrada;
            rowEntrada++;

        }
        font.setBold(true);
        font.setUnderline(true);
        painter.setFont(font);
        xPos = xPosParcela;
        yPos += 20;
        painter.drawText(xPos, yPos, "Devendo(R$):" + portugues.toString(devendoEntrada,'f',2));
        yPos += 20;
        font.setBold(false);
        font.setUnderline(false);
        painter.setFont(font);

    }

    yPos += 20;
    painter.drawText(xPosPrm, yPos, "Assinatura:" );
    yPos += 50;
    painter.drawText(xPosPrm, yPos, "Obrigado Pela Compra Volte Sempre!" );
    yPos += 30;

    painter.drawText(xPosPrm,yPos, "--");


    qDebug() << printer.pageLayout().pageSize();
    painter.end();

}
