#include "relatorios.h"
#include "ui_relatorios.h"
#include "mainwindow.h"
#include <QDebug>
//#include <QDebug>;


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
    float sumData4 = 0.0;
    while(query.next()){
        QString data2 = query.value(1).toString(); // quant

        QString data4 = query.value(3).toString(); // preco

        // double preco = portugues.toDouble(data4.toString());
        float valueData4 = data4.toDouble() * data2.toInt(); // Converte o valor para double
        sumData4 += valueData4; // Adiciona o valor à soma total


        ++row2;
    };
    // query.next();
    //  QString data4 = query.value(3).toString(); // preco
    // qDebug() <<  "DATA 4 =" + data4;

    painter.drawText(5000, 1000,"total R$:" + portugues.toString(sumData4));
    painter.drawText(8000, 1000,"total itens:" + QString::number( row2));

    QSqlQuery query2("SELECT * FROM produtos");




    int row = 1;
    //  double sumData4 = 0.0;

    QFontMetrics metrics(painter.font());
    while (query2.next()) {
        QString data1 = query2.value(0).toString(); // id
        QString data2 = query2.value(1).toString(); // quant
        QString data3 = query2.value(2).toString(); // desc
        QString data4 = query2.value(3).toString(); // preco
        QRect rect = metrics.boundingRect(QRect(0, 0, 4000, lineHeight), Qt::TextWordWrap, data3);
        int textHeight = rect.height();

        // Verifica se há espaço suficiente na página atual para desenhar outra linha
        if (startY + lineHeight * row > availableHeight) {
            // Se não houver, inicie uma nova página
            writer.newPage();
            startY = 100; // Reinicie a coordenada Y inicial
            row = 1; // Reinicie o contador de linha
        }

        // Desenhe os dados na página atual
        painter.drawText(QRect(1000, startY + lineHeight * row, 4000, textHeight), data1); //stary = 1500
        painter.drawText(QRect(1600, startY + lineHeight * row, 4000, textHeight), data2);
        painter.drawText(QRect(3000, startY + lineHeight * row, 4000, textHeight), Qt::TextWordWrap, data3); // data3 com quebra de linha
        painter.drawText(QRect(8500, startY + lineHeight * row, 4000, textHeight), portugues.toString(data4.toDouble()));

        startY += textHeight;

        ++row;
    }

    // // Desenha a quantidade de itens e a soma dos preços apenas na primeira página
    // painter.drawText(4000, 1000, "Quantidade de Itens: " + QString::number(totalItems));
    // painter.drawText(4000, 1100, "Soma dos preços: R$ " + QString::number(sumData4));

    painter.end();

    db.close();

    // Abre o PDF após a criação
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}


void relatorios::on_Btn_CsvGen_clicked(){

    QString fileName = QFileDialog::getSaveFileName(nullptr, "Salvar Arquivo CSV", "", "Arquivos CSV (*.csv)");

    if (fileName.isEmpty()) {
        // Se o usuário cancelar a seleção do arquivo, saia da função
        return;
    }

    // Abrindo o arquivo CSV para escrita
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Erro ao abrir o arquivo para escrita.";
        return;
    }
    QTextStream out(&file);

    if (!db.open()) {
        qDebug() << "Erro ao abrir o banco de dados botao csv.";
        return;
    }

    // Executando a consulta para recuperar os itens da tabela
    QSqlQuery query("SELECT * FROM produtos");
    out << "ID;Quant;Desc;Preço;CodBarra;NF\n";
    while (query.next()) {
        // Escrevendo os dados no arquivo CSV
        for (int i = 0; i < query.record().count(); ++i) {
            out << query.value(i).toString();
            if (i != query.record().count() - 1)
                out << ";"; // Adicionando ponto e vírgula para separar os campos
        }
        out << "\n"; // Adicionando uma nova linha após cada registro
    }

    // Fechando o arquivo e desconectando do banco de dados
    file.close();
    db.close();


}

