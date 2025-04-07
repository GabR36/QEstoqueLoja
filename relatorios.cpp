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

    ui->Stacked_Vendas->setCurrentIndex(0);
    ui->Stacked_Estoque->setCurrentIndex(0);


    configurarJanelaQuantVendas();

    connect(ui->CBox_VendasMain, QOverload<int>::of(&QComboBox::currentIndexChanged),
            ui->Stacked_Vendas, &QStackedWidget::setCurrentIndex);
    connect(ui->CBox_EstoqueMain, QOverload<int>::of(&QComboBox::currentIndexChanged),
            ui->Stacked_Estoque, &QStackedWidget::setCurrentIndex);

    configurarJanelaValorVendas();
    configurarJanelaTopProdutosVendas();





}







relatorios::~relatorios()
{
    delete ui;
}
void relatorios::configurarJanelaTopProdutosVendas(){
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

    QWidget* paginaGrafico = ui->Stacked_Vendas->widget(2); // página 3
    QLayout* layoutPagina = paginaGrafico->layout();


    if (!layoutPagina) {
        layoutPagina = new QVBoxLayout(paginaGrafico);
        paginaGrafico->setLayout(layoutPagina);
    }

    // Limpando e adicionando o gráfico na interface
    QLayoutItem *item;
    while ((item = layoutPagina->takeAt(2)) != nullptr) {
        delete item->widget();
        delete item;
    }
    layoutPagina->addWidget(chartView);

}
void relatorios::configurarJanelaValorVendas(){
    ui->CBox_MesValor->setVisible(false);
    ui->CBox_AnoValor->setVisible(false);

    ui->CBox_MesValor->addItems(meses);

    connect(ui->CBox_periodoValor, &QComboBox::currentTextChanged, this, [=](const QString &texto){
        if (texto == "Ano") {
            // Mostrar ComboBox de anos e buscar anos no banco de dados
            ui->CBox_AnoValor->setVisible(true);
            ui->CBox_MesValor->setVisible(false);
            ui->CBox_AnoValor->clear();
            ui->CBox_AnoValor->addItems(buscarAnosDisponiveis());
        }else if(texto == "Mes"){
            ui->CBox_AnoValor->setVisible(true);
            ui->CBox_MesValor->setVisible(true);
            ui->CBox_AnoValor->clear();
            ui->CBox_AnoValor->addItems(buscarAnosDisponiveis());
            emit ui->CBox_MesValor->currentTextChanged(ui->CBox_MesValor->currentText());

        }else {
            ui->CBox_AnoValor->setVisible(false);
            ui->CBox_MesValor->setVisible(false);

        }
    });

    connect(ui->CBox_AnoValor, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado) {
        if (ui->CBox_periodoValor->currentText() == "Ano") {
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

            QWidget* paginaGrafico = ui->Stacked_Vendas->widget(1); // página 0
            QLayout* layoutPagina = paginaGrafico->layout();


            if (!layoutPagina) {
                layoutPagina = new QVBoxLayout(paginaGrafico);
                paginaGrafico->setLayout(layoutPagina);
            }

            // Limpando e adicionando o gráfico na interface
            QLayoutItem *item;
            while ((item = layoutPagina->takeAt(2)) != nullptr) {
                delete item->widget();
                delete item;
            }
            layoutPagina->addWidget(chartView);
        }
    });

    connect(ui->CBox_MesValor, &QComboBox::currentTextChanged, this, [=](const QString &mesSelecionado){
        QString anoSelecionado = ui->CBox_AnoValor->currentText();
        if (ui->CBox_periodoValor->currentText() == "Mes" && !anoSelecionado.isEmpty()) {
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

            QWidget* paginaGrafico = ui->Stacked_Vendas->widget(1); // página 0
            QLayout* layoutPagina = paginaGrafico->layout();


            if (!layoutPagina) {
                layoutPagina = new QVBoxLayout(paginaGrafico);
                paginaGrafico->setLayout(layoutPagina);
            }
            // Limpando e adicionando o gráfico na página 0
            QLayoutItem *item;
            while ((item = layoutPagina->takeAt(2)) != nullptr) {
                delete item->widget();
                delete item;
            }


            layoutPagina->addWidget(chartView);
        }
    });
    ui->CBox_periodoValor->setCurrentIndex(1);
    emit ui->CBox_periodoValor->currentTextChanged(ui->CBox_periodoValor->currentText());

}
void relatorios::configurarJanelaQuantVendas(){

    ui->CBox_Ano->setVisible(false);
    ui->CBox_Mes->addItems(meses);
    ui->CBox_Mes->setVisible(false);

    connect(ui->CBox_Periodo, &QComboBox::currentTextChanged, this, [=](const QString &texto){
        if (texto == "Ano") {
            // Mostrar ComboBox de anos e buscar anos no banco de dados
            ui->CBox_Ano->setVisible(true);
            ui->CBox_Mes->setVisible(false);
            ui->CBox_Ano->clear();
            ui->CBox_Ano->addItems(buscarAnosDisponiveis());
        }else if(texto == "Mes"){
            ui->CBox_Ano->setVisible(true);
            ui->CBox_Mes->setVisible(true);
            ui->CBox_Ano->clear();
            ui->CBox_Ano->addItems(buscarAnosDisponiveis());
            emit ui->CBox_Mes->currentTextChanged(ui->CBox_Mes->currentText());

        }else {
            ui->CBox_Ano->setVisible(false);
            ui->CBox_Mes->setVisible(false);

        }
    });


    // Conectando o ComboBox de ano para atualizar o gráfico
    connect(ui->CBox_Ano, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado){
        if(ui->CBox_Periodo->currentText() == "Ano"){
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

            QWidget* paginaGrafico = ui->Stacked_Vendas->widget(0); // página 0
            QLayout* layoutPagina = paginaGrafico->layout();


            if (!layoutPagina) {
                layoutPagina = new QVBoxLayout(paginaGrafico);
                paginaGrafico->setLayout(layoutPagina);
            }

            // Limpando e adicionando o gráfico na página 0
            QLayoutItem *item;
            while ((item = layoutPagina->takeAt(2)) != nullptr) {
                delete item->widget();
                delete item;
            }
            layoutPagina->addWidget(chartView);
        }
    });
    ui->CBox_Periodo->setCurrentIndex(1);
    //emit ui->CBox_Periodo->currentTextChanged(ui->CBox_Periodo->currentText()); //emite ao abrir para resolver bugs

    connect( ui->CBox_Mes, &QComboBox::currentTextChanged, this, [=](const QString &mesSelecionado){
        QString anoSelecionado = ui->CBox_Ano->currentText();
        if (ui->CBox_Periodo->currentText() == "Mes" && !anoSelecionado.isEmpty()) {
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

            QWidget* paginaGrafico = ui->Stacked_Vendas->widget(0); // página 0
            QLayout* layoutPagina = paginaGrafico->layout();


            if (!layoutPagina) {
                layoutPagina = new QVBoxLayout(paginaGrafico);
                paginaGrafico->setLayout(layoutPagina);
            }

            // Limpando e adicionando o gráfico na página 0
            QLayoutItem *item;
            while ((item = layoutPagina->takeAt(2)) != nullptr) {
                delete item->widget();
                delete item;
            }
            layoutPagina->addWidget(chartView);
        }
    });
    ui->CBox_Periodo->setCurrentIndex(1);
    //emit ui->CBox_Ano->currentTextChanged(ui->CBox_Ano->currentText());

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

