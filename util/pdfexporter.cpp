#include "pdfexporter.h"
#include <QPdfWriter>
#include <QPainter>
#include <QSqlQuery>
#include <QDesktopServices>
#include <QLocale>
#include <QUrl>

PDFexporter::PDFexporter() {


}

void PDFexporter::exportarTodosProdutosParaPDF(const QString &fileName)
{
    QLocale portugues2;

    QSqlDatabase db = QSqlDatabase::database();

    if (fileName.isEmpty())
        return;

    if (!db.open()) {
        qDebug() << "nao abriu bd";
        return;
    }

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    QPainter painter(&writer);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setFont(QFont("Arial", 10, QFont::Bold));

    // Altura de linha e layout inicial
    int lineHeight = 300;
    int availableHeight = writer.height();
    int startY = 1500;

    // Desenho do cabeçalho
    QImage logo(":/QEstoqueLOja/mkaoyvbl.png");
    painter.drawImage(QRect(100, 100, 2000, 400), logo);
    painter.drawText(500, 1000, "Dados da Tabela Produtos:");
    painter.drawText(1000, 1500, "ID");
    painter.drawText(1600, 1500, "Quantidade");
    painter.drawText(3000, 1500, "Descrição");
    painter.drawText(8500, 1500, "Preço R$");

    QSqlQuery query("SELECT * FROM produtos");
    int row2 = 0;
    double sumData4 = 0.0;

    while (query.next()) {
        QString data2 = query.value(1).toString();
        QString data4 = query.value(3).toString();

        float soma = (data2.toFloat() < 0) ? 0 : data4.toDouble() * data2.toFloat();
        sumData4 += soma;
        ++row2;
    }

    painter.drawText(5000, 1000, "total R$:" + portugues2.toString(sumData4, 'f', 2));
    painter.drawText(8000, 1000, "total itens:" + QString::number(row2));

    QSqlQuery query2("SELECT * FROM produtos");
    int row = 1;
    QFontMetrics metrics(painter.font());

    while (query2.next()) {
        QString data1 = query2.value(0).toString();
        QString data2 = query2.value(1).toString();
        QString data3 = query2.value(2).toString();
        QString data4 = query2.value(3).toString();

        QRect rect = metrics.boundingRect(QRect(0, 0, 4000, lineHeight), Qt::TextWordWrap, data3);
        int textHeight = rect.height();

        if (startY + lineHeight * row > availableHeight) {
            writer.newPage();
            startY = 100;
            row = 1;
        }

        painter.drawText(QRect(1000, startY + lineHeight * row, 4000, textHeight), data1);
        painter.drawText(QRect(1600, startY + lineHeight * row, 4000, textHeight), portugues2.toString(data2.toDouble()));
        painter.drawText(QRect(3000, startY + lineHeight * row, 4000, textHeight), Qt::TextWordWrap, data3);
        painter.drawText(QRect(8500, startY + lineHeight * row, 4000, textHeight), portugues2.toString(data4.toDouble()));

        startY += textHeight;
        ++row;
    }

    painter.end();
    db.close();

    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}
void PDFexporter::exportarNfProdutosParaPDF(const QString &fileName){


    QSqlDatabase db = QSqlDatabase::database();
    QLocale portugues;

    if (fileName.isEmpty())
        return;

    if (!db.open()) {
        qDebug() << "nao abriu bd";
        return;
    }

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    QPainter painter(&writer);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setFont(QFont("Arial", 10, QFont::Bold));

    // Determinar a altura de uma linha e o espaço disponível na página
    int lineHeight = 300; // Altura de uma linha
    int availableHeight = writer.height(); // Altura disponível na página
    int startY = 1500; // Define a coordenada Y inicial

    // Desenha os dados da tabela no PDF
    QImage logo(":/QEstoqueLOja/mkaoyvbl.png");
    painter.drawImage(QRect(100, 100, 2000, 400), logo);
    painter.drawText(500, 1000,       "Dados da Tabela Produtos:");
    painter.drawText(1000, 1500,       "ID");
    painter.drawText(1600, 1500, "Quantidade");
    painter.drawText(3000, 1500, "Descrição");
    painter.drawText(8500, 1500, "Preço R$");

    QSqlQuery query("SELECT * FROM produtos");

    int row2 = 1;
    int rowNF = 1;
    float sumData4 = 0.0;
    while(query.next()){
        QString data2 = query.value(1).toString(); // quant
        QString data4 = query.value(3).toString(); // preco
        int variantNf = query.value(5).toInt();

        if(variantNf == 1){



            // double preco = portugues.toDouble(data4.toString());
            float soma;
            if(data2.toFloat() < 0){
                soma = data4.toDouble() * 0;
            }else{
                soma = data4.toDouble() * data2.toFloat(); // Converte o valor para double
            }
            sumData4 += soma; // Adiciona o valor à soma total
            ++rowNF;

        }

        ++row2;
    };

    painter.drawText(5000, 1000,"total R$:" + portugues.toString(sumData4,'f',2));
    painter.drawText(8000, 1000,"total itens:" + QString::number( rowNF));

    QSqlQuery query2("SELECT * FROM produtos");




    int row = 1;
    //  double sumData4 = 0.0;

    QFontMetrics metrics(painter.font());
    while (query2.next()) {
        QString data1 = query2.value(0).toString(); // id
        QString data2 = query2.value(1).toString(); // quant
        QString data3 = query2.value(2).toString(); // desc
        QString data4 = query2.value(3).toString(); // preco
        QString nf = query2.value(5).toString();



        QRect rect = metrics.boundingRect(QRect(0, 0, 4000, lineHeight), Qt::TextWordWrap, data3);
        int textHeight = rect.height();

        // Verifica se há espaço suficiente na página atual para desenhar outra linha
        if (startY + lineHeight * row > availableHeight) {
            // Se não houver, inicie uma nova página
            writer.newPage();
            startY = 100; // Reinicie a coordenada Y inicial
            row = 1; // Reinicie o contador de linha
        }

        if(nf == "1"){

            // Desenhe os dados na página atual
            painter.drawText(QRect(1000, startY + lineHeight * row, 4000, textHeight), data1); //stary = 1500
            painter.drawText(QRect(1600, startY + lineHeight * row, 4000, textHeight), portugues.toString(data2.toDouble()));
            painter.drawText(QRect(3000, startY + lineHeight * row, 4000, textHeight), Qt::TextWordWrap, data3); // data3 com quebra de linha
            painter.drawText(QRect(8500, startY + lineHeight * row, 4000, textHeight), portugues.toString(data4.toDouble()));

            startY += textHeight;

            ++row;
        }

    }
    qDebug() << "row = " + row;

    painter.end();

    db.close();

    // Abre o PDF após a criação
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

