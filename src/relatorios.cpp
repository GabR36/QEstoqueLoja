#include "relatorios.h"
#include "ui_relatorios.h"
#include "mainwindow.h"
#include <QDebug>
#include "util/pdfexporter.h"
#include <QBarSeries>
#include <QBarSet>
#include <QMap>
#include <QToolTip>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QFile>
#include <QTimer>
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

bool relatorios::existeProdutoVendido()
{
    return relatoriosServ.existeProdutoVendido();
}

void relatorios::configurarJanelaFormasPagamentoAno()
{
    if (ui->CBox_AnoFormaPagamento->count() == 0) {
        ui->CBox_AnoFormaPagamento->addItems(relatoriosServ.buscarAnosDisponiveis());
    }

    connect(ui->CBox_AnoFormaPagamento, &QComboBox::currentTextChanged, this, [=](const QString &ano){
        QString anoSelecionado = ui->CBox_AnoFormaPagamento->currentText();
        if (anoSelecionado.isEmpty()) return;

        QMap<QString, QVector<int>> dados = relatoriosServ.buscarFormasPagamentoPorAno(anoSelecionado);
        QStringList mesesAbrev = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez"};

        int maxValor = 0;
        QList<QBarSet*> sets;

        for (auto it = dados.begin(); it != dados.end(); ++it) {
            QBarSet *set = new QBarSet(it.key());
            for (int val : it.value()) {
                *set << val;
                maxValor = std::max(maxValor, val);
            }
            QString nomeForma = it.key();
            connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status) {
                    QToolTip::showText(QCursor::pos(), QString("%1: %2").arg(nomeForma).arg((*set)[index]));
                }
            });
            sets << set;
        }

        QChartView *chartView = GraficoHelper::criarBarChart(
            "Formas de Pagamento por Mês - Ano " + anoSelecionado,
            mesesAbrev, sets, maxValor);
        GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(3), chartView, 1);
    });

    emit ui->CBox_AnoFormaPagamento->currentTextChanged(ui->CBox_AnoFormaPagamento->currentText());
}

void relatorios::configurarJanelaTopProdutosVendas()
{
    QMap<QString, int> topProdutos = relatoriosServ.buscarTopProdutosVendidos();

    QBarSet *set = new QBarSet("Vendas");
    QStringList categorias;

    for (auto it = topProdutos.begin(); it != topProdutos.end(); ++it) {
        categorias << it.key();
        *set << it.value();
    }

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

    double maxY = topProdutos.isEmpty() ? 1.0 : *std::max_element(topProdutos.begin(), topProdutos.end());
    QChartView *chartView = GraficoHelper::criarBarChart("Top 10 Produtos Mais Vendidos", categorias, {set}, maxY);
    GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(2), chartView, 2);
}

void relatorios::configurarJanelaValorVendas()
{
    ui->CBox_MesValor->setVisible(false);
    ui->CBox_AnoValor->setVisible(false);

    ui->CBox_MesValor->addItems(meses);

    connect(ui->CBox_periodoValor, &QComboBox::currentTextChanged, this, [=](const QString &texto){
        if (texto == "Ano") {
            ui->CBox_AnoValor->setVisible(true);
            ui->CBox_MesValor->setVisible(false);
            ui->CBox_AnoValor->clear();
            ui->CBox_AnoValor->addItems(relatoriosServ.buscarAnosDisponiveis());
        } else if (texto == "Mes") {
            ui->CBox_AnoValor->setVisible(true);
            ui->CBox_MesValor->setVisible(true);
            ui->CBox_AnoValor->clear();
            ui->CBox_AnoValor->addItems(relatoriosServ.buscarAnosDisponiveis());
            emit ui->CBox_MesValor->currentTextChanged(ui->CBox_MesValor->currentText());
        } else {
            ui->CBox_AnoValor->setVisible(false);
            ui->CBox_MesValor->setVisible(false);
        }
    });

    connect(ui->CBox_AnoValor, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado) {
        if (ui->CBox_periodoValor->currentText() == "Ano") {
            QMap<QString, QPair<double, double>> vendasEEntradas = relatoriosServ.buscarValorVendasPorMesAno(anoSelecionado);

            QBarSet *setVendas = new QBarSet("Vendas");
            QBarSet *setEntradas = new QBarSet("Parcelas 'prazo'");
            QStringList categorias;
            double maxValor = 0;

            for (int i = 1; i <= 12; ++i) {
                QString mes = QString("%1").arg(i, 2, 10, QChar('0'));
                categorias << mes;

                double valorVendas = vendasEEntradas.value(mes, QPair<double, double>(0, 0)).first;
                double valorEntradas = vendasEEntradas.value(mes, QPair<double, double>(0, 0)).second;

                *setVendas << valorVendas;
                *setEntradas << valorEntradas;

                maxValor = std::max({maxValor, valorVendas, valorEntradas});
            }

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

            QChartView *chartView = GraficoHelper::criarBarChart(
                "Valor Vendas e Parcelas por Mês - Ano " + anoSelecionado,
                categorias, {setVendas, setEntradas}, maxValor * 1.1);
            GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(1), chartView, 2);
        }
    });

    connect(ui->CBox_MesValor, &QComboBox::currentTextChanged, this, [=](const QString &mesSelecionado) {
        QString anoSelecionado = ui->CBox_AnoValor->currentText();
        if (ui->CBox_periodoValor->currentText() == "Mes" && !anoSelecionado.isEmpty()) {
            QString mesFormatado = mesSelecionado.left(2);

            QMap<QString, double> vendas = relatoriosServ.buscarValorVendasPorDiaMesAno(anoSelecionado, mesFormatado);

            QBarSet *set = new QBarSet("Valor Total Vendas");
            QStringList categorias;

            int diasNoMes = QDate(anoSelecionado.toInt(), mesFormatado.toInt(), 1).daysInMonth();
            for (int dia = 1; dia <= diasNoMes; ++dia) {
                QString diaStr = QString("%1").arg(dia, 2, 10, QChar('0'));
                categorias << diaStr;
                *set << vendas.value(diaStr, 0.0);
            }

            connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status) {
                    QToolTip::showText(QCursor::pos(), QString("Valor: %1").arg((*set)[index]));
                }
            });

            double maxValor = vendas.isEmpty() ? 10.0 : *std::max_element(vendas.begin(), vendas.end());
            QChartView *chartView = GraficoHelper::criarBarChart(
                "Valor Vendas por Dia - " + mesSelecionado + " de " + anoSelecionado,
                categorias, {set}, maxValor);
            GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(1), chartView, 2);
        }
    });

    ui->CBox_periodoValor->setCurrentIndex(1);
}

void relatorios::configurarJanelaQuantVendas()
{
    ui->CBox_Ano->setVisible(false);
    ui->CBox_Mes->addItems(meses);
    ui->CBox_Mes->setVisible(false);

    connect(ui->CBox_Periodo, &QComboBox::currentTextChanged, this, [=](const QString &texto){
        if (texto == "Ano") {
            ui->CBox_Ano->setVisible(true);
            ui->CBox_Mes->setVisible(false);
            ui->CBox_Ano->clear();
            ui->CBox_Ano->addItems(relatoriosServ.buscarAnosDisponiveis());
        } else if (texto == "Mes") {
            ui->CBox_Ano->setVisible(true);
            ui->CBox_Mes->setVisible(true);
            ui->CBox_Ano->clear();
            ui->CBox_Ano->addItems(relatoriosServ.buscarAnosDisponiveis());
        } else {
            ui->CBox_Ano->setVisible(false);
            ui->CBox_Mes->setVisible(false);
        }
    });

    connect(ui->CBox_Ano, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado){
        if (ui->CBox_Periodo->currentText() == "Ano") {
            QMap<QString, int> vendas = relatoriosServ.buscarVendasPorMesAno(anoSelecionado);
            if (vendas.isEmpty()) {
                QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse ano.");
                return;
            }

            QBarSet *set = new QBarSet("Vendas");
            QStringList categorias;

            for (int i = 1; i <= 12; ++i) {
                QString mes = QString("%1").arg(i, 2, 10, QChar('0'));
                categorias << mes;
                *set << vendas.value(mes, 0);
            }

            connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status) {
                    QToolTip::showText(QCursor::pos(), QString("Valor: %1").arg((*set)[index]));
                }
            });

            double maxY = *std::max_element(vendas.begin(), vendas.end());
            QChartView *chartView = GraficoHelper::criarBarChart(
                "Quantidade de Vendas por Mês - Ano " + anoSelecionado,
                categorias, {set}, maxY);
            GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(0), chartView, 2);
        }
    });

    ui->CBox_Periodo->setCurrentIndex(1);

    connect(ui->CBox_Mes, &QComboBox::currentTextChanged, this, [=](const QString &mesSelecionado) {
        QString anoSelecionado = ui->CBox_Ano->currentText();
        if (ui->CBox_Periodo->currentText() == "Mes" && !anoSelecionado.isEmpty()) {
            QString mesFormatado = mesSelecionado.left(2);

            QMap<QString, int> vendas = relatoriosServ.buscarVendasPorDiaMesAno(anoSelecionado, mesFormatado);

            QBarSet *set = new QBarSet("Vendas");
            QStringList categorias;

            int diasNoMes = QDate(anoSelecionado.toInt(), mesFormatado.toInt(), 1).daysInMonth();
            for (int dia = 1; dia <= diasNoMes; ++dia) {
                QString diaStr = QString("%1").arg(dia, 2, 10, QChar('0'));
                categorias << diaStr;
                *set << vendas.value(diaStr, 0);
            }

            connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status) {
                    QToolTip::showText(QCursor::pos(), QString("Valor: %1").arg((*set)[index]));
                }
            });

            int maxVendas = vendas.isEmpty() ? 10 : *std::max_element(vendas.begin(), vendas.end());
            QChartView *chartView = GraficoHelper::criarBarChart(
                "Quantidade de Vendas por Dia - " + mesSelecionado + " de " + anoSelecionado,
                categorias, {set}, maxVendas);
            GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(0), chartView, 2);
        }
    });

    ui->CBox_Periodo->setCurrentIndex(1);
}

void relatorios::on_Btn_PdfGen_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Salvar PDF", QString(), "*.pdf");
    if (fileName.isEmpty())
        return;
    PDFexporter::exportarTodosProdutosParaPDF(fileName);
}

void relatorios::on_Btn_CsvGen_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Salvar Arquivo CSV", "", "Arquivos CSV (*.csv)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Erro ao abrir o arquivo para escrita.";
        return;
    }

    QTextStream out(&file);
    out << "ID;Quant;Desc;Preço;CodBarra;NF\n";

    QList<QStringList> produtos = relatoriosServ.buscarTodosProdutosParaCsv();
    for (const QStringList &row : produtos) {
        out << row.join(";") << "\n";
    }

    file.close();
}

void relatorios::configurarJanelaNFValor()
{
    ui->CBox_AnoNfValor->addItems(relatoriosServ.buscarAnosDisponiveis());

    connect(ui->CBox_AnoNfValor, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado){
        QMap<QString, float> valoresNf = relatoriosServ.buscarValoresNfAno(anoSelecionado, configDTO.tpAmbFiscal);
        if (valoresNf.isEmpty()) {
            QMessageBox::information(this, "Sem dados", "Não há Notas Fiscais registradas para esse ano.");
            return;
        }

        QBarSet *set = new QBarSet("Valor de Notas Fiscais emitidas");
        QStringList categorias;

        for (int i = 1; i <= 12; ++i) {
            QString mes = QString("%1").arg(i, 2, 10, QChar('0'));
            categorias << mes;
            *set << valoresNf.value(mes, 0);
        }

        connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
            if (status) {
                QToolTip::showText(QCursor::pos(), QString("Valor: %1").arg((*set)[index]));
            }
        });

        double maxY = *std::max_element(valoresNf.begin(), valoresNf.end());
        QChartView *chartView = GraficoHelper::criarBarChart(
            "Valor Emitididos em Nota Fiscal - Ano " + anoSelecionado,
            categorias, {set}, maxY);
        GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(4), chartView, 1);
    });

    emit ui->CBox_AnoNfValor->currentTextChanged(ui->CBox_AnoNfValor->currentText());
}

void relatorios::configurarJanelaProdutoLucroValor()
{
    ui->CBox_AnoProdutoLucro->addItems(relatoriosServ.buscarAnosDisponiveis());

    connect(ui->CBox_AnoProdutoLucro, &QComboBox::currentTextChanged, this, [=](const QString &anoSelecionado){
        QMap<QString, float> valoresProduto = relatoriosServ.produtosMaisLucrativosAno(anoSelecionado);
        if (valoresProduto.isEmpty()) {
            QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse ano.");
            return;
        }

        QBarSet *set = new QBarSet("Lucro");
        QStringList categorias;

        for (auto it = valoresProduto.constBegin(); it != valoresProduto.constEnd(); ++it) {
            categorias << it.key();
            *set << it.value();
        }

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

        double maxY = *std::max_element(valoresProduto.begin(), valoresProduto.end());
        QChartView *chartView = GraficoHelper::criarBarChart(
            "TOP 10 produtos que mais geraram lucro - Ano " + anoSelecionado,
            categorias, {set}, maxY);
        GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(5), chartView, 1);
    });

    emit ui->CBox_AnoProdutoLucro->currentTextChanged(ui->CBox_AnoProdutoLucro->currentText());
}
