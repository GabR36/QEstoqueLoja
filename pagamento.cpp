#include "pagamento.h"
#include "ui_pagamento.h"
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QDateTime>

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
    QDateTime dataIngles = portugues.toDateTime(dataGlobal, "dd-MM-yyyy hh:mm:ss");
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
    // tentar converter para float e ser menor ou igual ao valor final
    qDebug() << "validar valor desconto";
    qDebug() << desconto;
    bool menorQueTotal = portugues.toFloat(desconto, &conversionOkDesconto) <= portugues.toFloat(totalGlobal);
    qDebug() << conversionOkDesconto;
    if (!menorQueTotal){
        conversionOkDesconto = false;
    }
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

    query.prepare("INSERT INTO vendas2 (cliente, total, data_hora, forma_pagamento, valor_recebido, troco, taxa, valor_final, desconto) VALUES (:valor1, :valor2, :valor3, :valor4, :valor5, :valor6, :valor7, :valor8, :valor9)");
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
        query.bindValue(":valor2", QString::number(portugues.toInt(rowdata[1].toString())));
        if (query.exec()) {
            qDebug() << "update quantidade bem-sucedida!";
        } else {
            qDebug() << "Erro na update quantidade: ";
        }

    }
    if(ui->CheckImprimirCNF->isChecked()){
        QPrinter printer;
        //printer.setPageOrientation(QPageLayout::Portrait);
        //QSizeF customSize(80,297); // 204x842pt

        // customPageSize(customSize, QPageSize::Millimeter, "Customdasads");
        printer.setPageSize(QPageSize(QSizeF(80, 2000), QPageSize::Millimeter));// Tamanho do papel
       // printer.pageLayout().setPageSize(customPageSize);
        printer.setFullPage(true); // Utilizar toda a página        QPrintDialog dialog(&printer, this);

        QPrintDialog dialog(&printer, this);
        if(dialog.exec() == QDialog::Rejected) return;

        QPainter painter;
        painter.begin(&printer);
        QFont font = painter.font();
        font.setPointSize(8);
        font.setBold(true);
        painter.setFont(font);

        QString nomeEmpresa = "Padrão"; // pega os dados da configuração;
        QString enderecoEmpresa = "Padrão";
        QString cnpjEmpresa = "";
        QString telEmpresa = "";
        if (query.exec("SELECT value FROM config WHERE key = 'nome_empresa'")){
            while (query.next()) {
                nomeEmpresa = query.value(0).toString();
            }
        };
        if (query.exec("SELECT value FROM config WHERE key = 'endereco_empresa'")){
            while (query.next()) {
                enderecoEmpresa = query.value(0).toString();
            }
        };
        if (query.exec("SELECT value FROM config WHERE key = 'cnpj_empresa'")){
            while (query.next()) {
                cnpjEmpresa = query.value(0).toString();
            }
        };
        if (query.exec("SELECT value FROM config WHERE key = 'telefone_empresa'")){
            while (query.next()) {
                telEmpresa = query.value(0).toString();
            }// dataglobal , clienteglobal, totalglobal, desconte, forma de pagamento,recebido, taca e valorfinal?
        };




        int yPos = 30; // Posição inicial para começar a desenhar o texto
        int xPos = 0;
        const int yPosPrm = 10; // Posição inicial para começar a desenhar o texto
        const int xPosPrm = 10;
      //  painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText(95, 10, "Cupom Compra Venda");
        yPos += 20; // Avança a posição y
        painter.drawText(xPos, yPos, nomeEmpresa);
        yPos += 20;
        painter.drawText(xPos, yPos, enderecoEmpresa);
        yPos += 20;
        painter.drawText(xPos, yPos, cnpjEmpresa);
        yPos += 20;
        painter.drawText(xPos, yPos, telEmpresa);
        yPos += 20;
        painter.drawText(xPos, yPos, "Data/Hora: " + dataGlobal);
        yPos += 20;
        painter.drawText(xPos, yPos, "Cliente: " + clienteGlobal);
        yPos += 30;
        painter.drawText(xPos, yPos, "Quant:");
        int xPosProds = 45;
        xPos = xPosProds;
        painter.drawText(xPos, yPos, "Produtos vendidos:");
        int xPosValor = 202;
        xPos = xPosValor;
        painter.drawText(xPos, yPos, "ValorUn(R$):");
        yPos += 20;

        font.setPointSize(8);
        font.setBold(false);
        painter.setFont(font);
        //painter.setFont(QFont("Arial", 10));
        int lineHeight = 20; // Altura da linha
        int pageWidth = printer.pageLayout().paintRectPixels(printer.resolution()).width();
        for (const QList<QVariant> &rowdata : rowDataList) {
            QString idProduto = rowdata[0].toString();
            QString valorProduto = rowdata[3].toString();
            QString quantidadProduto = rowdata[1].toString();

            // Consultar a descrição do produto no banco de dados
            QSqlQuery queryProduto;
            queryProduto.prepare("SELECT descricao FROM produtos WHERE id = :idProduto");
            queryProduto.bindValue(":idProduto", idProduto);
            QString descricaoProduto = "Descrição não encontrada";
            if (queryProduto.exec()) {
                if (queryProduto.next()) {
                    descricaoProduto = queryProduto.value(0).toString();
                }
            } else {
                qDebug() << "Erro ao buscar descrição do produto: ";
            }
            QTextOption textOption;
            QRect rectQuantProd(xPosPrm,yPos, xPosProds ,lineHeight * 2 );
            painter.drawText(rectQuantProd, quantidadProduto, textOption);

            QRect rectDesc(xPosProds, yPos, pageWidth - 100, lineHeight * 2); // Definir um retângulo para o texto

            textOption.setWrapMode(QTextOption::WordWrap);
            painter.drawText(rectDesc, descricaoProduto, textOption);
             QRect rectValor(xPosValor + 30, yPos, pageWidth, lineHeight * 2);
            painter.drawText(rectValor, valorProduto,textOption);
            yPos += 30;

        }
        int posx = xPosPrm;
        for(int i=0; i < pageWidth; i++){
            posx += 3;
        painter.drawText(posx,yPos, "=");
        };
        font.setBold(true);
        painter.setFont(font);
         yPos += 20;
    //    painter.drawText(Qt::AlignCenter,yPos, "Pagamento");
        xPos = 95;
        painter.drawText(xPos,yPos, "Desconto(R$): " + desconto);
        yPos += 20;
        painter.drawText(xPos,yPos, "Forma Pagamento: " + forma_pagamento);
        yPos += 20;
        painter.drawText(xPos,yPos, "Valor Total Produtos(R$): " + totalGlobal);
        yPos += 20;
        switch (ui->CBox_FormaPagamento->currentIndex()){
        case 0: //dinehiro
            painter.drawText(xPos, yPos, "Valor Recebido(R$):" + recebido);
            yPos += 20;
            painter.drawText(xPos,yPos, "Troco(R$):" + troco);
            break;
        case 1: //nsei
            break;
        case 2: //credito
            painter.drawText(xPos, yPos, "Taxa(%):" + taxa);
            yPos += 20;
            painter.drawText(xPos, yPos, "Valor Final(%):" + valor_final);
            break;
        case 3: //debito
            painter.drawText(xPos, yPos, "Taxa(%):" + taxa);
            yPos += 20;
            painter.drawText(xPos, yPos, "Valor Final(%):" + valor_final);
            break;
        case 4: //pix
            break;

        default: break;
        };
        yPos += 20;
        painter.drawText(xPosPrm, yPos, "Assinatura:" );
        yPos += 50;
         painter.drawText(xPosPrm, yPos, "Obrigado Pela Compra Volte Sempre!" );
        yPos += 30;

        painter.drawText(xPosPrm,yPos, "--");


        qDebug() << printer.pageLayout().pageSize();
        painter.end();
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
    QString valorFinal = ui->Lbl_TotalTaxa->text();
    float troco = portugues.toFloat(dinRecebido) - portugues.toFloat(valorFinal);

    ui->Lbl_Troco->setText(portugues.toString(troco, 'f', 2));

}


void pagamento::on_CBox_FormaPagamento_activated(int index)
{
    // mostrar ou esconder campos relacionados ao troco
    // a depender da forma dinheiro ser selecionada

    // pegar os valores de taxas padrao das configuracoes
    if(!janelaVenda->db.open()){
        qDebug() << "erro ao abrir banco de dados. pagamento combobox";
    }
    QSqlQuery query;

    QString taxaDebito  = "3";
    QString taxaCredito = "4";
    if (query.exec("SELECT value FROM config WHERE key = 'taxa_debito'")){
        while (query.next()) {
            taxaDebito = portugues.toString(query.value(0).toFloat());
        }
    }
    if (query.exec("SELECT value FROM config WHERE key = 'taxa_credito'")){
        while (query.next()) {
            taxaCredito = portugues.toString(query.value(0).toFloat());
        }
    }
    //

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
        ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(taxaCredito, "0"), 'f', 2));
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
        ui->Lbl_TotalTaxa->setText(portugues.toString(obterValorFinal(taxaDebito, "0"), 'f', 2));
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
    descontoTaxa();
}

float pagamento::obterValorFinal(QString taxa, QString desconto){
    float valorFinal = (portugues.toFloat(totalGlobal) - portugues.toFloat(desconto)) * (1 + portugues.toFloat(taxa)/100);
    return valorFinal;
}

void pagamento::on_Ledit_Desconto_textChanged(const QString &arg1)
{
    descontoTaxa();
}

void pagamento::descontoTaxa(){
    QString novaTaxa = ui->Ledit_Taxa->text();
    QString desconto = ui->Ledit_Desconto->text();
    QString valorFinal = portugues.toString(obterValorFinal(novaTaxa, desconto), 'f', 2);
    ui->Lbl_TotalTaxa->setText(valorFinal);
    // o valor final influencia os campos recebido, portanto modificacoes nele devem afetar
    // os campos influenciados por ele

    // o valor recebido deve ser o mesmo que o valor final, pois ao alterar o valor final
    // o dinheiro recebido não será mais o mesmo do valor final anterior
    ui->Ledit_Recebido->setText(valorFinal);

    // o troco será zero portanto
    ui->Lbl_Troco->setText("0");
}

