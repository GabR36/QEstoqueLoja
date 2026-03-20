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
#include <QMessageBox>
#include <QStandardItemModel>
#include <QFile>
#include <QSqlQuery>
#include <QTimer>
#include <QSqlError>
#include <QSqlRecord>
#include <QSql>
#include "configuracao.h"
#include <QStandardPaths>

relatorios::relatorios(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::relatorios)
{
    ui->setupUi(this);
    Config_service *confServ = new Config_service(this);
    configDTO = confServ->carregarTudo();

    connect(ui->CBox_VendasMain, QOverload<int>::of(&QComboBox::currentIndexChanged),
            ui->Stacked_Vendas, &QStackedWidget::setCurrentIndex);

    if (existeProdutoVendido()) {
        db.open();
        configurarJanelaQuantVendas();
        configurarJanelaValorVendas();
        configurarJanelaTopProdutosVendas();
        configurarJanelaFormasPagamentoAno();
        configurarJanelaNFValor();
        configurarJanelaProdutoLucroValor();
    }

    ui->CBox_VendasMain->setCurrentIndex(0);
    ui->Stacked_Vendas->setCurrentIndex(0);
}


relatorios::~relatorios()
{
    delete ui;
}
QMap<QString, QVector<int>> relatorios::buscarFormasPagamentoPorAno(const QString &anoSelecionado) {
    QMap<QString, QVector<int>> resultado;

    // Inicializa com 12 zeros para cada forma
    QStringList formas = {"Dinheiro", "Crédito", "Débito", "Pix", "Prazo", "Não Sei"};
    for (const QString &forma : formas) {
        resultado[forma] = QVector<int>(12, 0);
    }

    QSqlQuery query;
    query.prepare(R"(
        SELECT strftime('%m', data_hora) AS mes, forma_pagamento, COUNT(*) as total
        FROM vendas2
        WHERE strftime('%Y', data_hora) = :ano
        GROUP BY mes, forma_pagamento
    )");
    query.bindValue(":ano", anoSelecionado);

    if (query.exec()) {
        while (query.next()) {
            int mes = query.value(0).toString().toInt(); // 1 a 12
            QString forma = query.value(1).toString();
            int total = query.value(2).toInt();

            if (resultado.contains(forma)) {
                resultado[forma][mes - 1] = total;
            }
        }
    } else {
        qDebug() << "Erro ao buscar formas pagamento por ano:" << query.lastError().text();
    }

    return resultado;
}
void relatorios::configurarJanelaFormasPagamentoAno() {

    // Carrega os anos apenas se o combo estiver vazio
    if (ui->CBox_AnoFormaPagamento->count() == 0) {
        ui->CBox_AnoFormaPagamento->addItems(buscarAnosDisponiveis());
    }

    // Conecta o combo apenas uma vez

        connect(ui->CBox_AnoFormaPagamento, &QComboBox::currentTextChanged, this, [=](const QString &ano){
           // qDebug() << "Combo mudou para ano:" << ano;

            // Pega o ano selecionado e monta o gráfico
            QString anoSelecionado = ui->CBox_AnoFormaPagamento->currentText();
            if (anoSelecionado.isEmpty()) return;

            QMap<QString, QVector<int>> dados = buscarFormasPagamentoPorAno(anoSelecionado);
            QStringList meses = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez"};

            QChart *chart = new QChart();
            chart->setTitle("Formas de Pagamento por Mês - Ano " + anoSelecionado);
            chart->setAnimationOptions(QChart::SeriesAnimations);

            QBarSeries *series = new QBarSeries();
            int maxValor = 0;

            for (auto it = dados.begin(); it != dados.end(); ++it) {
                QBarSet *set = new QBarSet(it.key());
                for (int val : it.value()) {
                    *set << val;
                    maxValor = std::max(maxValor, val);
                }
                QString nomeForma = it.key();  // Captura fora da lambda

                connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
                    if (status) {
                        QToolTip::showText(QCursor::pos(), QString("%1: %2").arg(nomeForma).arg((*set)[index]));
                    }
                });

                series->append(set);
            }

            chart->addSeries(series);

            QBarCategoryAxis *axisX = new QBarCategoryAxis();
            axisX->append(meses);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);

            QValueAxis *axisY = new QValueAxis();
            axisY->setRange(0, maxValor);
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            QChartView *chartView = new QChartView(chart);
            chartView->setRenderHint(QPainter::Antialiasing);

            QWidget* paginaGrafico = ui->Stacked_Vendas->widget(3);
            QLayout* layoutPagina = paginaGrafico->layout();

            if (!layoutPagina) {
                layoutPagina = new QVBoxLayout(paginaGrafico);
                paginaGrafico->setLayout(layoutPagina);
            }

            // Remove widgets anteriores (mantendo os dois primeiros)
            QLayoutItem *item;
            while ((item = layoutPagina->takeAt(1)) != nullptr) {
                delete item->widget();
                delete item;
            }
            layoutPagina->addWidget(chartView);
        });

        emit ui->CBox_AnoFormaPagamento->currentTextChanged(ui->CBox_AnoFormaPagamento->currentText());




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
            QToolTip::showText(
                QCursor::pos(),
                QString("%1\nValor: %2")
                    .arg(categorias.at(index))
                    .arg((*set)[index], 0, 'f', 2)
                );
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
            // if (vendasEEntradas.isEmpty()) {
            //     QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse ano.");
            //     return; // ou pode limpar o gráfico, se quiser
            // }
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

    connect(ui->CBox_MesValor, &QComboBox::currentTextChanged, this, [=](const QString &mesSelecionado) {
        QString anoSelecionado = ui->CBox_AnoValor->currentText();
        if (ui->CBox_periodoValor->currentText() == "Mes" && !anoSelecionado.isEmpty()) {
            QString mesFormatado = mesSelecionado.left(2);  // Pega o número do mês

            QMap<QString, double> vendas = buscarValorVendasPorDiaMesAno(anoSelecionado, mesFormatado);

            // Criando o gráfico de barras para vendas diárias
            QBarSet *set = new QBarSet("Valor Total Vendas");
            QStringList categorias;

            int diasNoMes = QDate(anoSelecionado.toInt(), mesFormatado.toInt(), 1).daysInMonth();
            for (int dia = 1; dia <= diasNoMes; ++dia) {
                QString diaStr = QString("%1").arg(dia, 2, 10, QChar('0'));
                categorias << diaStr;
                *set << vendas.value(diaStr, 0.0);  // Preenche com 0.0 se não houver valor
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

            double maxValor = vendas.isEmpty() ? 10.0 : *std::max_element(vendas.begin(), vendas.end());
            axisY->setRange(0, maxValor);
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            QChartView *chartView = new QChartView(chart);
            chartView->setRenderHint(QPainter::Antialiasing);

            QWidget* paginaGrafico = ui->Stacked_Vendas->widget(1); // página 1
            QLayout* layoutPagina = paginaGrafico->layout();

            if (!layoutPagina) {
                layoutPagina = new QVBoxLayout(paginaGrafico);
                paginaGrafico->setLayout(layoutPagina);
            }

            // Limpando e adicionando o gráfico na página 1
            QLayoutItem *item;
            while ((item = layoutPagina->takeAt(2)) != nullptr) {
                delete item->widget();
                delete item;
            }

            layoutPagina->addWidget(chartView);
        }
    });
    ui->CBox_periodoValor->setCurrentIndex(1);
    //emit ui->CBox_periodoValor->currentTextChanged(ui->CBox_periodoValor->currentText());

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
           // emit ui->CBox_Mes->currentTextChanged(ui->CBox_Mes->currentText());

        }else {
            ui->CBox_Ano->setVisible(false);
            ui->CBox_Mes->setVisible(false);

        }
    });


    // Conectando o ComboBox de ano para atualizar o gráfico
    connect(ui->CBox_Ano, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado){
        if(ui->CBox_Periodo->currentText() == "Ano"){
            QMap<QString, int> vendas = buscarVendasPorMesAno(anoSelecionado);
            if (vendas.isEmpty()) {
                QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse ano.");
                return; // ou pode limpar o gráfico, se quiser
            }
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

    connect(ui->CBox_Mes, &QComboBox::currentTextChanged, this, [=](const QString &mesSelecionado) {
        QString anoSelecionado = ui->CBox_Ano->currentText();
        if (ui->CBox_Periodo->currentText() == "Mes" && !anoSelecionado.isEmpty()) {
            QString mesFormatado = mesSelecionado.left(2);

            QMap<QString, int> vendas = buscarVendasPorDiaMesAno(anoSelecionado, mesFormatado);

            QStringList categorias;
            QBarSet *set = new QBarSet("Vendas");

            int diasNoMes = QDate(anoSelecionado.toInt(), mesFormatado.toInt(), 1).daysInMonth();
            for (int dia = 1; dia <= diasNoMes; ++dia) {
                QString diaStr = QString("%1").arg(dia, 2, 10, QChar('0'));
                categorias << diaStr;
                *set << vendas.value(diaStr, 0);  // Usa 0 se não existir venda no dia
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

            int maxVendas = vendas.isEmpty() ? 10 : *std::max_element(vendas.begin(), vendas.end());
            axisY->setRange(0, maxVendas);
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            QChartView *chartView = new QChartView(chart);
            chartView->setRenderHint(QPainter::Antialiasing);

            QWidget *paginaGrafico = ui->Stacked_Vendas->widget(0);
            QLayout *layoutPagina = paginaGrafico->layout();

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
bool relatorios::existeProdutoVendido(){
    if(!db.open()){
        qDebug() << "Erro ao abrir o banco de dados";
        return false;
    }

    QSqlQuery query;
    if(query.exec("SELECT 1 FROM produtos_vendidos LIMIT 1")) {
        if(query.next()) {
            return true; // Existe pelo menos um produto vendido
        }
    } else {
        qDebug() << "Erro na consulta:" << query.lastError().text();
    }

    return false; // Nenhum produto encontrado ou erro na consulta
}




QMap<QString, float> relatorios::buscarValoresNfAno(const QString &ano) {

    QMap<QString, float> valores;
    qDebug() << ano;
    QSqlQuery query;
    query.prepare("SELECT strftime('%m', dhemi) AS mes, SUM(valor_total) "
                  "FROM notas_fiscais "
                  "WHERE (strftime('%Y', dhemi) = :ano "
                  "AND (cstat = '100' OR cstat = '150')) AND tp_amb = :tpamb "
                  "AND finalidade = 'NORMAL' "
                  "GROUP BY mes");
    query.bindValue(":ano", ano);
    query.bindValue(":tpamb", configDTO.tpAmbFiscal);
    qDebug() << "tpamb: " << configDTO.tpAmbFiscal;

    if (query.exec()) {
        while (query.next()) {
            QString mes = query.value(0).toString(); // "01", "02", ..., "12"
            float valor = query.value(1).toFloat();
            valores[mes] = valor;
        }
    } else {
        qDebug() << "Erro ao buscar valores das NFs por ano:" << query.lastError().text();
    }

    return valores;
}

void relatorios::configurarJanelaNFValor(){

    ui->CBox_AnoNfValor->addItems(buscarAnosDisponiveis());
    // Conectando o ComboBox de ano para atualizar o gráfico
    connect(ui->CBox_AnoNfValor, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado){
            QMap<QString, float> valoresNf = buscarValoresNfAno(anoSelecionado);
            if (valoresNf.isEmpty()) {
                QMessageBox::information(this, "Sem dados", "Não há Notas Fiscais registradas para esse ano.");
                return; // ou pode limpar o gráfico, se quiser
            }
            // Criando o gráfico de barras
            QBarSet *set = new QBarSet("Valor de Notas Fiscais emitidas");
            QStringList categorias;

            for (int i = 1; i <= 12; ++i) {
                QString mes = QString("%1").arg(i, 2, 10, QChar('0'));
                categorias << mes;
                *set << valoresNf.value(mes, 0);
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
            chart->setTitle("Valor Emitididos em Nota Fiscal - Ano " + anoSelecionado);
            chart->setAnimationOptions(QChart::SeriesAnimations);

            QBarCategoryAxis *axisX = new QBarCategoryAxis();
            axisX->append(categorias);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);

            QValueAxis *axisY = new QValueAxis();
            axisY->setRange(0, *std::max_element(valoresNf.begin(), valoresNf.end()));
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            QChartView *chartView = new QChartView(chart);
            chartView->setRenderHint(QPainter::Antialiasing);

            QWidget* paginaGrafico = ui->Stacked_Vendas->widget(4); // página
            QLayout* layoutPagina = paginaGrafico->layout();


            if (!layoutPagina) {
                layoutPagina = new QVBoxLayout(paginaGrafico);
                paginaGrafico->setLayout(layoutPagina);
            }

            // Limpando e adicionando o gráfico na página 0
            QLayoutItem *item;
            while ((item = layoutPagina->takeAt(1)) != nullptr) {
                delete item->widget();
                delete item;
            }
            layoutPagina->addWidget(chartView);

    });
     emit ui->CBox_AnoNfValor->currentTextChanged(ui->CBox_AnoNfValor->currentText());


}

QMap<QString, float> relatorios::produtosMaisLucrativosAno(const QString &ano) {
    QMap<QString, float> produtosLucro;

    QSqlQuery query;
    query.prepare(R"(
        SELECT
            p.descricao,
            SUM(
                pv.quantidade *
                CASE
                    WHEN p.preco_fornecedor > 0 AND p.preco_fornecedor != '' THEN
                        (pv.preco_vendido - p.preco_fornecedor)
                    ELSE
                        (pv.preco_vendido * (p.porcent_lucro / 100.0))
                END
            ) AS lucro_total
        FROM produtos_vendidos pv
        JOIN produtos p ON pv.id_produto = p.id
        JOIN vendas2 v ON pv.id_venda = v.id
        WHERE strftime('%Y', v.data_hora) = :ano
        GROUP BY p.descricao
        HAVING lucro_total > 0
        ORDER BY lucro_total DESC
        LIMIT 10
    )");

    query.bindValue(":ano", ano);

    if (query.exec()) {
        while (query.next()) {
            produtosLucro.insert(
                query.value(0).toString(),
                query.value(1).toFloat()
                );
        }
    } else {
        qDebug() << "Erro ao buscar produtos mais lucrativos:" << query.lastError().text();
    }

    return produtosLucro;
}



void relatorios::configurarJanelaProdutoLucroValor(){
    ui->CBox_AnoProdutoLucro->addItems(buscarAnosDisponiveis());
    // Conectando o ComboBox de ano para atualizar o gráfico
    connect(ui->CBox_AnoProdutoLucro, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado){
        QMap<QString, float> valoresProduto = produtosMaisLucrativosAno(anoSelecionado);
        if (valoresProduto.isEmpty()) {
            QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse ano.");
            return; // ou pode limpar o gráfico, se quiser
        }
        // Criando o gráfico de barras
        QBarSet *set = new QBarSet("Lucro");
        QStringList categorias;

        for (auto it = valoresProduto.constBegin(); it != valoresProduto.constEnd(); ++it) {
            categorias << it.key();         // nome do produto
            *set << it.value();             // lucro
        }


        QBarSeries *series = new QBarSeries();
        series->append(set);

        connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
            if (status) {

                QToolTip::showText(
                    QCursor::pos(),
                    QString("%1\nLucro: R$ %2")
                        .arg(categorias.at(index))
                        .arg((*set)[index], 0, 'f', 2)
                    );
            }
        });

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("TOP 10 produtos que mais geraram lucro - Ano " + anoSelecionado);
        chart->setAnimationOptions(QChart::SeriesAnimations);

        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categorias);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0, *std::max_element(valoresProduto.begin(), valoresProduto.end()));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        QWidget* paginaGrafico = ui->Stacked_Vendas->widget(5); // página
        QLayout* layoutPagina = paginaGrafico->layout();


        if (!layoutPagina) {
            layoutPagina = new QVBoxLayout(paginaGrafico);
            paginaGrafico->setLayout(layoutPagina);
        }

        // Limpando e adicionando o gráfico na página 0
        QLayoutItem *item;
        while ((item = layoutPagina->takeAt(1)) != nullptr) {
            delete item->widget();
            delete item;
        }
        layoutPagina->addWidget(chartView);

    });
    emit ui->CBox_AnoProdutoLucro->currentTextChanged(ui->CBox_AnoProdutoLucro->currentText());
}
