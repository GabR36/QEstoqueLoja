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
#include <QToolTip>
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

    QLabel *Lbl_Periodo = new QLabel();
    Lbl_Periodo->setText("Período:");
    QVBoxLayout *layoutPeriodo = new QVBoxLayout();
    layoutPeriodo->addWidget(Lbl_Periodo);
    layoutPeriodo->addWidget(CBox_Periodo);

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
    QWidget *paginaQuantVendas = new QWidget();
    QVBoxLayout *layoutGrafico = new QVBoxLayout();
    QHBoxLayout *layoutAnoMes = new QHBoxLayout();
    layoutAnoMes->addWidget(CBox_Ano);
    layoutAnoMes->addWidget(CBox_Mes);

    layoutGrafico->addLayout(layoutPeriodo);
    layoutGrafico->addLayout(layoutAnoMes);
    paginaQuantVendas->setLayout(layoutGrafico);

    // Adicionando a página 0 ao StackedWidget
    ui->Stacked_Vendas->addWidget(paginaQuantVendas);

    // Adicionando uma página vazia como segunda página
    QWidget *paginaProdVendidos = new QWidget();
    QVBoxLayout *layoutProdVendidos = new QVBoxLayout();
    paginaProdVendidos->setLayout(layoutProdVendidos);
    ui->Stacked_Vendas->addWidget(paginaProdVendidos);

    QWidget *paginaValorVendas = new QWidget();
    QVBoxLayout *layoutVendasValor = new QVBoxLayout();
    QVBoxLayout *layoutPeriodoValor= new QVBoxLayout();


    QLabel *Lbl_PeriodoValor = new QLabel();
    Lbl_PeriodoValor->setText("Periodo");

    QComboBox *CBox_periodoValor = new QComboBox();

    QHBoxLayout *layoutMesAnoValor = new QHBoxLayout();


    QComboBox *CBox_MesValor = new QComboBox();
    QComboBox *CBox_AnoValor = new QComboBox();
    CBox_MesValor->setVisible(false);
    CBox_AnoValor->setVisible(false);


    CBox_MesValor->addItems(meses);

    layoutMesAnoValor->addWidget(CBox_MesValor);
    layoutMesAnoValor->addWidget(CBox_AnoValor);



    CBox_periodoValor->addItem("Mes");
    CBox_periodoValor->addItem("Ano");

    layoutPeriodoValor->addWidget(Lbl_PeriodoValor);
    layoutPeriodoValor->addWidget(CBox_periodoValor);

    layoutVendasValor->addLayout(layoutPeriodoValor);
    layoutVendasValor->addLayout(layoutMesAnoValor);

    paginaValorVendas->setLayout(layoutVendasValor);
    ui->Stacked_Vendas->addWidget(paginaValorVendas);


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
            emit CBox_Mes->currentTextChanged(CBox_Mes->currentText());

        }else {
            CBox_Ano->setVisible(false);
            CBox_Mes->setVisible(false);

        }
    });

    // Conectando o ComboBox de período
    connect(CBox_periodoValor, &QComboBox::currentTextChanged, this, [=](const QString &texto){
        if (texto == "Ano") {
            // Mostrar ComboBox de anos e buscar anos no banco de dados
            CBox_AnoValor->setVisible(true);
            CBox_MesValor->setVisible(false);
            CBox_AnoValor->clear();
            CBox_AnoValor->addItems(buscarAnosDisponiveis());
        }else if(texto == "Mes"){
            CBox_AnoValor->setVisible(true);
            CBox_MesValor->setVisible(true);
            CBox_AnoValor->clear();
            CBox_AnoValor->addItems(buscarAnosDisponiveis());
            emit CBox_MesValor->currentTextChanged(CBox_MesValor->currentText());

        }else {
            CBox_AnoValor->setVisible(false);
            CBox_MesValor->setVisible(false);

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

            connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status) {
                    QToolTip::showText(QCursor::pos(), QString("Valor: %1").arg((*set)[index]));
                }
            });

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

    // Grafico de valor por Ano
    connect(CBox_AnoValor, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado) {
        if (CBox_periodoValor->currentText() == "Ano") {
            QMap<QString, QPair<double, double>> vendasEEntradas = buscarValorVendasPorMesAno(anoSelecionado);

            // Criando conjuntos de dados para o gráfico
            QBarSet *setVendas = new QBarSet("Vendas");
            QBarSet *setEntradas = new QBarSet("Parcelas 'prazo'");
            QStringList categorias;

            double maxValor = 0; // Para definir o limite do eixo Y

            for (int i = 1; i <= 12; ++i) {
                QString mes = QString("%1").arg(i, 2, 10, QChar('0'));
                categorias << mes;

                double valorVendas = vendasEEntradas.value(mes, QPair<double, double>(0, 0)).first;
                double valorEntradas = vendasEEntradas.value(mes, QPair<double, double>(0, 0)).second;

                *setVendas << valorVendas;
                *setEntradas << valorEntradas;

                maxValor = std::max({maxValor, valorVendas, valorEntradas});
            }

            // Criando a série do gráfico e adicionando os conjuntos de dados
            QBarSeries *series = new QBarSeries();
            series->append(setVendas);
            series->append(setEntradas);

            connect(setVendas, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status) {
                    QToolTip::showText(QCursor::pos(), QString("R$: %1").arg((*setVendas)[index]));
                }
            });
            connect(setEntradas, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status) {
                    QToolTip::showText(QCursor::pos(), QString("R$: %1").arg((*setEntradas)[index]));
                }
            });

            // Criando o gráfico
            QChart *chart = new QChart();
            chart->addSeries(series);
            chart->setTitle("Valor Vendas e Parcelas por Mês - Ano " + anoSelecionado);
            chart->setAnimationOptions(QChart::SeriesAnimations);

            // Eixo X (Meses)
            QBarCategoryAxis *axisX = new QBarCategoryAxis();
            axisX->append(categorias);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);

            // Eixo Y (Valores)
            QValueAxis *axisY = new QValueAxis();
            axisY->setRange(0, maxValor * 1.1); // Adiciona 10% ao valor máximo para melhor visualização
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            // Criando o QChartView
            QChartView *chartView = new QChartView(chart);
            chartView->setRenderHint(QPainter::Antialiasing);

            // Limpando e adicionando o gráfico na interface
            QLayoutItem *item;
            while ((item = layoutPeriodoValor->takeAt(2)) != nullptr) {
                delete item->widget();
                delete item;
            }
            layoutPeriodoValor->addWidget(chartView);
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
            connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status) {
                    QToolTip::showText(QCursor::pos(), QString("Valor: %1").arg((*set)[index]));
                }
            });

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

    // grafico VALOR mes - ano
    connect(CBox_MesValor, &QComboBox::currentTextChanged, this, [=](const QString &mesSelecionado){
        QString anoSelecionado = CBox_AnoValor->currentText();
        if (CBox_periodoValor->currentText() == "Mes" && !anoSelecionado.isEmpty()) {
            QString mesFormatado = mesSelecionado.left(2);  // Pega o número do mês

            QMap<QString, double> vendas = buscarValorVendasPorDiaMesAno(anoSelecionado, mesFormatado); //<<<<<<

            // Criando o gráfico de barras para vendas diárias
            QBarSet *set = new QBarSet("Valor Total Vendas");
            QStringList categorias;

            int diasNoMes = QDate(anoSelecionado.toInt(), mesFormatado.toInt(), 1).daysInMonth();
            for (int dia = 1; dia <= diasNoMes; ++dia) {
                QString diaStr = QString("%1").arg(dia, 2, 10, QChar('0'));
                categorias << diaStr;
                *set << vendas.value(diaStr, 0);
            }

            QBarSeries *series = new QBarSeries();
            series->append(set);
            connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status) {
                    QToolTip::showText(QCursor::pos(), QString("Valor: %1").arg((*set)[index]));
                }
            });

            QChart *chart = new QChart();
            chart->addSeries(series);
            chart->setTitle("Valor Vendas por Dia - " + mesSelecionado + " de " + anoSelecionado);
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
            while ((item = layoutPeriodoValor->takeAt(2)) != nullptr) {
                delete item->widget();
                delete item;
            }
            layoutPeriodoValor->addWidget(chartView);
        }
    });

    CBox_Periodo->setCurrentIndex(1);
    emit CBox_Periodo->currentTextChanged(CBox_Periodo->currentText());
    CBox_periodoValor->setCurrentIndex(1);
    emit CBox_periodoValor->currentTextChanged(CBox_periodoValor->currentText());


    // Conectando o ComboBox principal para alternar páginas
    connect(ui->CBox_VendasMain, QOverload<int>::of(&QComboBox::currentIndexChanged),
            ui->Stacked_Vendas, &QStackedWidget::setCurrentIndex);

    QMap<QString, int> topProdutos = buscarTopProdutosVendidos();

    // Criando o gráfico de barras para os produtos mais vendidos
    QBarSet *set = new QBarSet("Vendas");
    QStringList categorias;

    for (auto it = topProdutos.begin(); it != topProdutos.end(); ++it) {
        categorias << it.key();
        *set << it.value();
    }

    QBarSeries *series = new QBarSeries();
    series->append(set);
    connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
        if (status) {
            QToolTip::showText(QCursor::pos(), QString("Valor: %1").arg((*set)[index]));
        }
    });

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Top 10 Produtos Mais Vendidos");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categorias);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, *std::max_element(topProdutos.begin(), topProdutos.end()));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    layoutProdVendidos->addWidget(chartView);
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


QMap<QString, double> relatorios::buscarValorVendasPorDiaMesAno(const QString& ano, const QString& mes) {
    QMap<QString, double> vendasPorDia;
    QSqlQuery query;
    query.prepare(R"(
        SELECT strftime('%d', data_hora) AS dia, SUM(valor_final) AS total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = :ano
          AND strftime('%m', data_hora) = :mes
          AND forma_pagamento != 'Prazo'
        GROUP BY dia
    )");

    query.bindValue(":ano", ano);
    query.bindValue(":mes", mes);

    if (query.exec()) {
        while (query.next()) {
            QString dia = query.value(0).toString();
            double total = query.value(1).toDouble();
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

QMap<QString, QPair<double, double>> relatorios::buscarValorVendasPorMesAno(const QString& ano) {
    QMap<QString, QPair<double, double>> totalPorMes;
    QSqlQuery query;

    query.prepare(R"(
        SELECT v.mes,
               COALESCE(v.total_vendas, 0) AS total_vendas,
               COALESCE(e.total_entradas, 0) AS total_entradas
        FROM
        (
            SELECT strftime('%m', data_hora) AS mes, SUM(valor_final) AS total_vendas
            FROM vendas2
            WHERE strftime('%Y', data_hora) = :ano AND forma_pagamento != 'Prazo'
            GROUP BY mes
        ) AS v
        LEFT JOIN
        (
            SELECT strftime('%m', data_hora) AS mes, SUM(valor_final) AS total_entradas
            FROM entradas_vendas
            WHERE strftime('%Y', data_hora) = :ano
            GROUP BY mes
        ) AS e
        ON v.mes = e.mes
    )");

    query.bindValue(":ano", ano);

    if (query.exec()) {
        while (query.next()) {
            QString mes = query.value(0).toString();
            double totalVendas = query.value(1).toDouble();
            double totalEntradas = query.value(2).toDouble();
            totalPorMes[mes] = QPair<double, double>(totalVendas, totalEntradas);
        }
    } else {
        qDebug() << "Erro ao buscar vendas e entradas:" << query.lastError().text();
    }

    return totalPorMes;
}

QMap<QString, int> relatorios::buscarTopProdutosVendidos() {
    QMap<QString, int> topProdutos;
    QSqlQuery query;
    query.prepare(R"(
        SELECT p.descricao, SUM(pv.quantidade) AS total
        FROM produtos_vendidos pv
        JOIN produtos p ON pv.id_produto = p.id
        GROUP BY p.descricao
        ORDER BY total DESC
        LIMIT 10
    )");

    if (query.exec()) {
        while (query.next()) {
            QString produto = query.value(0).toString();
            int total = query.value(1).toInt();
            topProdutos[produto] = total;
        }
    } else {
        qDebug() << "Erro ao buscar top produtos:" << query.lastError().text();
    }

    return topProdutos;
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

