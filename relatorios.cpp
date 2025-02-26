#include "relatorios.h"
#include "ui_relatorios.h"
#include "mainwindow.h"
#include <QDebug>
#include "util/pdfexporter.h"
#include <QChart>
#include <QPieSeries>
#include <QChartView>
//#include <QDebug>;

relatorios::relatorios(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::relatorios)
{

    ui->setupUi(this);

    while (ui->Stacked_Vendas->count() > 0) {
        QWidget *pagina = ui->Stacked_Vendas->widget(0);
        ui->Stacked_Vendas->removeWidget(pagina);
        pagina->deleteLater(); // Libera a memória
    }
    // Criando o gráfico
    QChart *chartVenda = new QChart();
    QPieSeries *seriesPeriodo = new QPieSeries();
    seriesPeriodo->append("Janeiro", 30);
    seriesPeriodo->append("Fevereiro", 40);
    seriesPeriodo->append("Março", 50);
    chartVenda->addSeries(seriesPeriodo);
    chartVenda->setTitle("Vendas por Mês");

    QChartView *chartViewPeriodo = new QChartView(chartVenda);
    chartViewPeriodo->setRenderHint(QPainter::Antialiasing);

    // IMPORTANTE: Adicione primeiro o gráfico, depois a página vazia
    ui->Stacked_Vendas->addWidget(chartViewPeriodo);

    QWidget *paginaVazia = new QWidget();
    ui->Stacked_Vendas->addWidget(paginaVazia);

    // Definindo o índice inicial para mostrar o gráfico
    ui->Stacked_Vendas->setCurrentIndex(0);

    // Conectando o ComboBox ao StackedWidget
    connect(ui->CBox_VendasMain, QOverload<int>::of(&QComboBox::currentIndexChanged),
            ui->Stacked_Vendas, &QStackedWidget::setCurrentIndex);

    // Debug para verificar os índices
    qDebug() << "Total de páginas no Stacked_Vendas:" << ui->Stacked_Vendas->count();
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
    PDFexporter::exportarTodosProdutosParaPDF(fileName);
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

