#include "relatorios.h"
#include "ui_relatorios.h"
//#include <QDebug>;
#include <QPainter>
#include <QFileDialog>
#include <QPdfWriter>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QtSql>
#include <QDesktopServices>

relatorios::relatorios(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::relatorios)
{
    ui->setupUi(this);
}

relatorios::~relatorios()
{
    delete ui;
}

void relatorios::on_Btn_PdfGen_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Salvar PDF", QString(), "*.pdf");
    if (fileName.isEmpty())
        return;

    if (!db.open()) {
        qDebug() << "nao abriu bd";
        return;
    }

    QSqlQuery query("SELECT * FROM produtos");

    QPdfWriter writer(fileName);
    QPainter painter(&writer);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setFont(QFont("Arial", 10, QFont::Bold));

    // Desenha os dados da tabela no PDF

    QImage logo(":/QEstoqueLOja/mkaoyvbl.png");
    //painter.drawImage(QRect(300, 600, 400, 800), logo);
    painter.drawText(500, 1000,       "Dados da Tabela Produtos:");
    painter.drawText(1000, 1500,       "ID");
    painter.drawText(1600, 1500, "Quantidade");
    painter.drawText(3000, 1500, "Descrição");
    painter.drawText(7000, 1500, "Preço");
    int row = 1;
    painter.setFont(QFont("Arial", 10));
    int lineHeight = 500; // Define o espaçamento vertical entre as linhas
    int startY = 2000; // Define a coordenada Y inicial
    while (query.next()) {
        QString data1 = query.value(0).toString(); // Coluna 1
        QString data2 = query.value(1).toString(); // Coluna 2
        QString data3 = query.value(2).toString();
        QString data4 = query.value(3).toString();
        painter.drawText(1000, startY + lineHeight * row, data1);
        painter.drawText(1600, startY + lineHeight * row, data2);
        painter.drawText(3000, startY + lineHeight * row, data3);
        painter.drawText(7000, startY + lineHeight * row, "R$ " + data4);

        ++row;
    }


    painter.end();
    db.close();

    // Abre o PDF após a criação
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));


}


