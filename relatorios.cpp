#include "relatorios.h"
#include "ui_relatorios.h"
#include "mainwindow.h"
#include <QDebug>
#include "util/pdfexporter.h"
#include <QChart>
#include <QPieSeries>
#include <QBarSeries>
#include <QChartView>
#include <QBarCategoryAxis>
#include <QBarSet>
#include <QMap>
#include <QValueAxis>
//#include <QDebug>;

relatorios::relatorios(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::relatorios)
{
    ui->setupUi(this);

    // Limpando páginas antigas do StackedWidget
    while (ui->Stacked_Vendas->count() > 0) {
        QWidget *pagina = ui->Stacked_Vendas->widget(0);
        ui->Stacked_Vendas->removeWidget(pagina);
        pagina->deleteLater();
    }

    // Criando o ComboBox para selecionar o período
    QComboBox *CBox_Periodo = new QComboBox();
    CBox_Periodo->addItem("Mes");
    CBox_Periodo->addItem("Ano");

    // Criando o ComboBox para selecionar o ano (inicialmente oculto)
    QComboBox *CBox_Ano = new QComboBox();
    CBox_Ano->setVisible(false);
    QComboBox *CBox_Mes = new QComboBox();
    QStringList meses = {"01 - Janeiro", "02 - Fevereiro", "03 - Março", "04 - Abril", "05 - Maio",
                         "06 - Junho", "07 - Julho", "08 - Agosto", "09 - Setembro",
                         "10 - Outubro", "11 - Novembro", "12 - Dezembro"};
    CBox_Mes->addItems(meses);
    CBox_Mes->setVisible(false);

    // Criando o container da página 0
    QWidget *paginaGrafico = new QWidget();
    QVBoxLayout *layoutGrafico = new QVBoxLayout();
    QHBoxLayout *layoutAnoMes = new QHBoxLayout();
    layoutAnoMes->addWidget(CBox_Ano);
    layoutAnoMes->addWidget(CBox_Mes);

    layoutGrafico->addWidget(CBox_Periodo);
    layoutGrafico->addLayout(layoutAnoMes);
    paginaGrafico->setLayout(layoutGrafico);

    // Adicionando a página 0 ao StackedWidget
    ui->Stacked_Vendas->addWidget(paginaGrafico);

    // Adicionando uma página vazia como segunda página
    QWidget *paginaVazia = new QWidget();
    ui->Stacked_Vendas->addWidget(paginaVazia);

    // Definindo o índice inicial para mostrar o gráfico e o ComboBox juntos
    ui->Stacked_Vendas->setCurrentIndex(0);

    // Conectando o ComboBox de período
    connect(CBox_Periodo, &QComboBox::currentTextChanged, this, [=](const QString &texto){
        if (texto == "Ano") {
            // Mostrar ComboBox de anos e buscar anos no banco de dados
            CBox_Ano->setVisible(true);
            CBox_Mes->setVisible(false);
            CBox_Ano->clear();
            CBox_Ano->addItems(buscarAnosDisponiveis());
        }else if(texto == "Mes"){
            CBox_Ano->setVisible(true);
            CBox_Mes->setVisible(true);
            CBox_Ano->clear();
            CBox_Ano->addItems(buscarAnosDisponiveis());
        }else {
            CBox_Ano->setVisible(false);
            CBox_Mes->setVisible(false);

        }
    });

    // Conectando o ComboBox de ano para atualizar o gráfico
    connect(CBox_Ano, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado){
        if(CBox_Periodo->currentText() == "Ano"){
            QMap<QString, int> vendas = buscarVendasPorMesAno(anoSelecionado);

            // Criando o gráfico de barras
            QBarSet *set = new QBarSet("Vendas");
            QStringList categorias;

            for (int i = 1; i <= 12; ++i) {
                QString mes = QString("%1").arg(i, 2, 10, QChar('0'));
                categorias << mes;
                *set << vendas.value(mes, 0);
            }

            QBarSeries *series = new QBarSeries();
            series->append(set);

            QChart *chart = new QChart();
            chart->addSeries(series);
            chart->setTitle("Quantidade de Vendas por Mês - Ano " + anoSelecionado);
            chart->setAnimationOptions(QChart::SeriesAnimations);

            QBarCategoryAxis *axisX = new QBarCategoryAxis();
            axisX->append(categorias);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);

            QValueAxis *axisY = new QValueAxis();
            axisY->setRange(0, *std::max_element(vendas.begin(), vendas.end()));
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            QChartView *chartView = new QChartView(chart);
            chartView->setRenderHint(QPainter::Antialiasing);

            // Limpando e adicionando o gráfico na página 0
            QLayoutItem *item;
            while ((item = layoutGrafico->takeAt(2)) != nullptr) {
                delete item->widget();
                delete item;
            }
            layoutGrafico->addWidget(chartView);
        }
    });

    connect(CBox_Mes, &QComboBox::currentTextChanged, this, [=](const QString &mesSelecionado){
        QString anoSelecionado = CBox_Ano->currentText();
        if (CBox_Periodo->currentText() == "Mes" && !anoSelecionado.isEmpty()) {
            QString mesFormatado = mesSelecionado.left(2);  // Pega o número do mês

            QMap<QString, int> vendas = buscarVendasPorDiaMesAno(anoSelecionado, mesFormatado);

            // Criando o gráfico de barras para vendas diárias
            QBarSet *set = new QBarSet("Vendas");
            QStringList categorias;

            int diasNoMes = QDate(anoSelecionado.toInt(), mesFormatado.toInt(), 1).daysInMonth();
            for (int dia = 1; dia <= diasNoMes; ++dia) {
                QString diaStr = QString("%1").arg(dia, 2, 10, QChar('0'));
                categorias << diaStr;
                *set << vendas.value(diaStr, 0);
            }

            QBarSeries *series = new QBarSeries();
            series->append(set);

            QChart *chart = new QChart();
            chart->addSeries(series);
            chart->setTitle("Quantidade de Vendas por Dia - " + mesSelecionado + " de " + anoSelecionado);
            chart->setAnimationOptions(QChart::SeriesAnimations);

            QBarCategoryAxis *axisX = new QBarCategoryAxis();
            axisX->append(categorias);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);

            QValueAxis *axisY = new QValueAxis();
            axisY->setRange(0, *std::max_element(vendas.begin(), vendas.end()));
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            QChartView *chartView = new QChartView(chart);
            chartView->setRenderHint(QPainter::Antialiasing);

            // Limpando e adicionando o gráfico na página 0
            QLayoutItem *item;
            while ((item = layoutGrafico->takeAt(2)) != nullptr) {
                delete item->widget();
                delete item;
            }
            layoutGrafico->addWidget(chartView);
        }
    });

    CBox_Periodo->setCurrentIndex(1);
    emit CBox_Periodo->currentTextChanged(CBox_Periodo->currentText());

    // Conectando o ComboBox principal para alternar páginas
    connect(ui->CBox_VendasMain, QOverload<int>::of(&QComboBox::currentIndexChanged),
            ui->Stacked_Vendas, &QStackedWidget::setCurrentIndex);
}






relatorios::~relatorios()
{
    delete ui;
}
void relatorios::conectarBancoDados() {
    if (!db.open()) {
        qDebug() << "Erro ao conectar ao banco de dados:" << db.lastError().text();
    } else {
        qDebug() << "Conectado ao banco de dados.";
    }
}
QStringList relatorios::buscarAnosDisponiveis() {
    QStringList anos;
    QSqlQuery query;
    query.prepare(R"(
        SELECT DISTINCT strftime('%Y', data_hora) AS ano
        FROM vendas2
        ORDER BY ano DESC
    )");

    if (query.exec()) {
        while (query.next()) {
            anos << query.value(0).toString();
        }
    } else {
        qDebug() << "Erro ao buscar anos:" << query.lastError().text();
    }

    return anos;
}

QMap<QString, int> relatorios::buscarVendasPorMes() {
    QMap<QString, int> vendasPorMes;

    QSqlQuery query;
    query.prepare(R"(
        SELECT strftime('%m', data_hora) AS mes, COUNT(*) AS total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = strftime('%Y', 'now')
        GROUP BY mes
    )");

    if (query.exec()) {
        while (query.next()) {
            QString mes = query.value(0).toString();
            int total = query.value(1).toInt();
            vendasPorMes[mes] = total;
        }
    } else {
        qDebug() << "Erro na consulta:" << query.lastError().text();
    }

    return vendasPorMes;
}
QMap<QString, int> relatorios::buscarVendasPorDiaMesAno(const QString& ano, const QString& mes) {
    QMap<QString, int> vendasPorDia;
    QSqlQuery query;
    query.prepare(R"(
        SELECT strftime('%d', data_hora) AS dia, COUNT(*) AS total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = :ano AND strftime('%m', data_hora) = :mes
        GROUP BY dia
    )");
    query.bindValue(":ano", ano);
    query.bindValue(":mes", mes);

    if (query.exec()) {
        while (query.next()) {
            QString dia = query.value(0).toString();
            int total = query.value(1).toInt();
            vendasPorDia[dia] = total;
        }
    } else {
        qDebug() << "Erro ao buscar vendas por dia:" << query.lastError().text();
    }

    return vendasPorDia;
}



QMap<QString, int> relatorios::buscarVendasPorMesAno(const QString& ano) {
    QMap<QString, int> vendasPorMes;
    QSqlQuery query;
    query.prepare(R"(
        SELECT strftime('%m', data_hora) AS mes, COUNT(*) AS total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = :ano
        GROUP BY mes
    )");
    query.bindValue(":ano", ano);

    if (query.exec()) {
        while (query.next()) {
            QString mes = query.value(0).toString();
            int total = query.value(1).toInt();
            vendasPorMes[mes] = total;
        }
    } else {
        qDebug() << "Erro ao buscar vendas:" << query.lastError().text();
    }

    return vendasPorMes;
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

