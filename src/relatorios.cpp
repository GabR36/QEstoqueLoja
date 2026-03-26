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
#include <QSet>
#include <QComboBox>
#include <QChartView>
#include "util/pdfexporter.h"
#include "util/csvexporter.h"

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

    connect(ui->Btn_ExportCSV, &QPushButton::clicked, this, &relatorios::exportarCsvAtual);
    connect(ui->Btn_ExportPDF, &QPushButton::clicked, this, &relatorios::exportarPdfAtual);

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

Agrupamento relatorios::agrupFromCombo(QComboBox *cb, bool semDia)
{
    QString t = cb->currentText();
    if (!semDia && t == "Dia") return Agrupamento::Dia;
    if (t == "Ano")            return Agrupamento::Ano;
    return Agrupamento::Mes;
}

void relatorios::configurarJanelaQuantVendas()
{
    QDate inicio = QDate(QDate::currentDate().year(), 1, 1);
    QDate fim    = QDate::currentDate();
    ui->DE_InicioQuant->setDate(inicio);
    ui->DE_FimQuant->setDate(fim);
    ui->CBox_AgrupQuant->setCurrentIndex(1); // Mês por padrão

    connect(ui->Btn_AplicarQuant, &QPushButton::clicked, this, [=]() {
        QDate de  = ui->DE_InicioQuant->date();
        QDate ate = ui->DE_FimQuant->date();
        if (de > ate) {
            QMessageBox::warning(this, "Período inválido", "A data inicial não pode ser posterior à data final.");
            return;
        }

        Agrupamento agrup = agrupFromCombo(ui->CBox_AgrupQuant);
        QMap<QString, int> dados = relatoriosServ.buscarQuantVendasPeriodo(de, ate, agrup);
        if (dados.isEmpty()) {
            QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse período.");
            return;
        }

        QStringList categorias = dados.keys();
        QBarSet *set = new QBarSet("Vendas");
        for (const QString &k : categorias)
            *set << dados[k];

        connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
            if (status)
                QToolTip::showText(QCursor::pos(), QString("Vendas: %1").arg((*set)[index]));
        });

        double maxY = *std::max_element(dados.begin(), dados.end());
        QString titulo = QString("Quantidade de Vendas por %1 (%2 – %3)")
                         .arg(ui->CBox_AgrupQuant->currentText())
                         .arg(de.toString("dd/MM/yyyy"))
                         .arg(ate.toString("dd/MM/yyyy"));
        QChartView *cv = GraficoHelper::criarBarChart(titulo, categorias, {set}, maxY);
        GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(0), cv, 1);
    });

    emit ui->Btn_AplicarQuant->clicked();
}

void relatorios::configurarJanelaValorVendas()
{
    QDate inicio = QDate(QDate::currentDate().year(), 1, 1);
    QDate fim    = QDate::currentDate();
    ui->DE_InicioValor->setDate(inicio);
    ui->DE_FimValor->setDate(fim);
    ui->CBox_AgrupValor->setCurrentIndex(1); // Mês por padrão

    connect(ui->Btn_AplicarValor, &QPushButton::clicked, this, [=]() {
        QDate de  = ui->DE_InicioValor->date();
        QDate ate = ui->DE_FimValor->date();
        if (de > ate) {
            QMessageBox::warning(this, "Período inválido", "A data inicial não pode ser posterior à data final.");
            return;
        }

        Agrupamento agrup = agrupFromCombo(ui->CBox_AgrupValor);
        QMap<QString, QPair<double,double>> dados = relatoriosServ.buscarValorVendasPeriodo(de, ate, agrup);
        if (dados.isEmpty()) {
            QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse período.");
            return;
        }

        QStringList categorias = dados.keys();
        QBarSet *setVendas   = new QBarSet("Vendas");
        QBarSet *setEntradas = new QBarSet("Prazo");
        double maxValor = 0;

        for (const QString &k : categorias) {
            double v = dados[k].first;
            double e = dados[k].second;
            *setVendas   << v;
            *setEntradas << e;
            maxValor = std::max({maxValor, v, e});
        }

        connect(setVendas, &QBarSet::hovered, this, [=](bool status, int index) {
            if (status)
                QToolTip::showText(QCursor::pos(), QString("R$: %1").arg((*setVendas)[index]));
        });
        connect(setEntradas, &QBarSet::hovered, this, [=](bool status, int index) {
            if (status)
                QToolTip::showText(QCursor::pos(), QString("R$: %1").arg((*setEntradas)[index]));
        });

        QString titulo = QString("Valor de Vendas por %1 (%2 – %3)")
                         .arg(ui->CBox_AgrupValor->currentText())
                         .arg(de.toString("dd/MM/yyyy"))
                         .arg(ate.toString("dd/MM/yyyy"));
        QChartView *cv = GraficoHelper::criarBarChart(titulo, categorias, {setVendas, setEntradas}, maxValor * 1.1);
        GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(1), cv, 1);
    });

    emit ui->Btn_AplicarValor->clicked();
}

void relatorios::configurarJanelaTopProdutosVendas()
{
    QDate inicio = QDate(QDate::currentDate().year(), 1, 1);
    QDate fim    = QDate::currentDate();
    ui->DE_InicioTop->setDate(inicio);
    ui->DE_FimTop->setDate(fim);

    connect(ui->Btn_AplicarTop, &QPushButton::clicked, this, [=]() {
        QDate de  = ui->DE_InicioTop->date();
        QDate ate = ui->DE_FimTop->date();
        if (de > ate) {
            QMessageBox::warning(this, "Período inválido", "A data inicial não pode ser posterior à data final.");
            return;
        }

        QMap<QString, int> dados = relatoriosServ.buscarTopProdutosVendidosPeriodo(de, ate);
        if (dados.isEmpty()) {
            QMessageBox::information(this, "Sem dados", "Não há produtos vendidos para esse período.");
            return;
        }

        QStringList categorias = dados.keys();
        QBarSet *set = new QBarSet("Vendas");
        for (const QString &k : categorias)
            *set << dados[k];

        connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
            if (status)
                QToolTip::showText(QCursor::pos(),
                    QString("%1\nQtd: %2").arg(categorias.at(index)).arg((*set)[index]));
        });

        double maxY = *std::max_element(dados.begin(), dados.end());
        QString titulo = QString("Top 10 Produtos Mais Vendidos (%1 – %2)")
                         .arg(de.toString("dd/MM/yyyy"))
                         .arg(ate.toString("dd/MM/yyyy"));
        QChartView *cv = GraficoHelper::criarBarChart(titulo, categorias, {set}, maxY);
        GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(2), cv, 1);
    });

    emit ui->Btn_AplicarTop->clicked();
}

void relatorios::configurarJanelaFormasPagamentoAno()
{
    QDate inicio = QDate(QDate::currentDate().year(), 1, 1);
    QDate fim    = QDate::currentDate();
    ui->DE_InicioFormas->setDate(inicio);
    ui->DE_FimFormas->setDate(fim);
    ui->CBox_AgrupFormas->setCurrentIndex(0); // Mês por padrão

    connect(ui->Btn_AplicarFormas, &QPushButton::clicked, this, [=]() {
        QDate de  = ui->DE_InicioFormas->date();
        QDate ate = ui->DE_FimFormas->date();
        if (de > ate) {
            QMessageBox::warning(this, "Período inválido", "A data inicial não pode ser posterior à data final.");
            return;
        }

        Agrupamento agrup = agrupFromCombo(ui->CBox_AgrupFormas, /*semDia=*/true);
        QMap<QString, QMap<QString,int>> dados = relatoriosServ.buscarFormasPagamentoPeriodo(de, ate, agrup);
        if (dados.isEmpty()) {
            QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse período.");
            return;
        }

        // Coletar todos os períodos únicos em ordem
        QSet<QString> periodSet;
        for (auto &inner : dados)
            for (auto it = inner.cbegin(); it != inner.cend(); ++it)
                periodSet.insert(it.key());
        QStringList categorias = periodSet.values();
        categorias.sort();

        int maxValor = 0;
        QList<QBarSet*> sets;
        for (auto it = dados.cbegin(); it != dados.cend(); ++it) {
            QBarSet *set = new QBarSet(it.key());
            for (const QString &p : categorias) {
                int val = it.value().value(p, 0);
                *set << val;
                maxValor = std::max(maxValor, val);
            }
            QString nomeForma = it.key();
            connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
                if (status)
                    QToolTip::showText(QCursor::pos(),
                        QString("%1: %2").arg(nomeForma).arg((*set)[index]));
            });
            sets << set;
        }

        QString titulo = QString("Formas de Pagamento por %1 (%2 – %3)")
                         .arg(ui->CBox_AgrupFormas->currentText())
                         .arg(de.toString("dd/MM/yyyy"))
                         .arg(ate.toString("dd/MM/yyyy"));
        QChartView *cv = GraficoHelper::criarBarChart(titulo, categorias, sets, maxValor);
        GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(3), cv, 1);
    });

    emit ui->Btn_AplicarFormas->clicked();
}

void relatorios::configurarJanelaNFValor()
{
    QDate inicio = QDate(QDate::currentDate().year(), 1, 1);
    QDate fim    = QDate::currentDate();
    ui->DE_InicioNF->setDate(inicio);
    ui->DE_FimNF->setDate(fim);
    ui->CBox_AgrupNF->setCurrentIndex(0); // Mês por padrão

    connect(ui->Btn_AplicarNF, &QPushButton::clicked, this, [=]() {
        QDate de  = ui->DE_InicioNF->date();
        QDate ate = ui->DE_FimNF->date();
        if (de > ate) {
            QMessageBox::warning(this, "Período inválido", "A data inicial não pode ser posterior à data final.");
            return;
        }

        Agrupamento agrup = agrupFromCombo(ui->CBox_AgrupNF, /*semDia=*/true);
        QMap<QString, float> dados = relatoriosServ.buscarValoresNfPeriodo(de, ate, agrup, configDTO.tpAmbFiscal);
        if (dados.isEmpty()) {
            QMessageBox::information(this, "Sem dados", "Não há Notas Fiscais registradas para esse período.");
            return;
        }

        QStringList categorias = dados.keys();
        QBarSet *set = new QBarSet("Valor de Notas Fiscais emitidas");
        for (const QString &k : categorias)
            *set << dados[k];

        connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
            if (status)
                QToolTip::showText(QCursor::pos(), QString("Valor: %1").arg((*set)[index]));
        });

        double maxY = *std::max_element(dados.begin(), dados.end());
        QString titulo = QString("Valor Emitido em Nota Fiscal por %1 (%2 – %3)")
                         .arg(ui->CBox_AgrupNF->currentText())
                         .arg(de.toString("dd/MM/yyyy"))
                         .arg(ate.toString("dd/MM/yyyy"));
        QChartView *cv = GraficoHelper::criarBarChart(titulo, categorias, {set}, maxY);
        GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(4), cv, 1);
    });

    emit ui->Btn_AplicarNF->clicked();
}

void relatorios::configurarJanelaProdutoLucroValor()
{
    QDate inicio = QDate(QDate::currentDate().year(), 1, 1);
    QDate fim    = QDate::currentDate();
    ui->DE_InicioLucro->setDate(inicio);
    ui->DE_FimLucro->setDate(fim);

    connect(ui->Btn_AplicarLucro, &QPushButton::clicked, this, [=]() {
        QDate de  = ui->DE_InicioLucro->date();
        QDate ate = ui->DE_FimLucro->date();
        if (de > ate) {
            QMessageBox::warning(this, "Período inválido", "A data inicial não pode ser posterior à data final.");
            return;
        }

        QMap<QString, float> dados = relatoriosServ.produtosMaisLucrativosPeriodo(de, ate);
        if (dados.isEmpty()) {
            QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse período.");
            return;
        }

        QStringList categorias;
        QBarSet *set = new QBarSet("Lucro");
        for (auto it = dados.cbegin(); it != dados.cend(); ++it) {
            categorias << it.key();
            *set << it.value();
        }

        connect(set, &QBarSet::hovered, this, [=](bool status, int index) {
            if (status)
                QToolTip::showText(QCursor::pos(),
                    QString("%1\nLucro: R$ %2")
                        .arg(categorias.at(index))
                        .arg((*set)[index], 0, 'f', 2));
        });

        double maxY = *std::max_element(dados.begin(), dados.end());
        QString titulo = QString("TOP 10 Produtos que mais geraram Lucro (%1 – %2)")
                         .arg(de.toString("dd/MM/yyyy"))
                         .arg(ate.toString("dd/MM/yyyy"));
        QChartView *cv = GraficoHelper::criarBarChart(titulo, categorias, {set}, maxY);
        GraficoHelper::inserirChartNaPagina(ui->Stacked_Vendas->widget(5), cv, 1);
    });

    emit ui->Btn_AplicarLucro->clicked();
}

// ── Helpers de exportação ──────────────────────────────────────────────────

QChartView *relatorios::chartViewAtual()
{
    QWidget *page = ui->Stacked_Vendas->currentWidget();
    if (!page || !page->layout() || page->layout()->count() < 2)
        return nullptr;
    return qobject_cast<QChartView*>(page->layout()->itemAt(1)->widget());
}

void relatorios::exportarPdfAtual()
{
    PDFexporter::exportarGraficoRelatorio(this, chartViewAtual());
}

void relatorios::exportarCsvAtual()
{
    int idx = ui->Stacked_Vendas->currentIndex();
    QList<QStringList> linhas;
    QString nomeArquivo = "relatorio.csv";

    switch (idx) {
    case 0: { // Quantidade de vendas
        auto dados = relatoriosServ.buscarQuantVendasPeriodo(
            ui->DE_InicioQuant->date(), ui->DE_FimQuant->date(),
            agrupFromCombo(ui->CBox_AgrupQuant));
        linhas << QStringList{"Período", "Quantidade de Vendas"};
        for (auto it = dados.cbegin(); it != dados.cend(); ++it)
            linhas << QStringList{it.key(), QString::number(it.value())};
        nomeArquivo = "relatorio_quant_vendas.csv";
        break;
    }
    case 1: { // Valor de vendas
        auto dados = relatoriosServ.buscarValorVendasPeriodo(
            ui->DE_InicioValor->date(), ui->DE_FimValor->date(),
            agrupFromCombo(ui->CBox_AgrupValor));
        linhas << QStringList{"Período", "Vendas (R$)", "Valor Prazo (R$)"};
        for (auto it = dados.cbegin(); it != dados.cend(); ++it)
            linhas << QStringList{it.key(),
                                  QString::number(it.value().first,  'f', 2),
                                  QString::number(it.value().second, 'f', 2)};
        nomeArquivo = "relatorio_valor_vendas.csv";
        break;
    }
    case 2: { // Top produtos
        auto dados = relatoriosServ.buscarTopProdutosVendidosPeriodo(
            ui->DE_InicioTop->date(), ui->DE_FimTop->date());
        linhas << QStringList{"Produto", "Quantidade Vendida"};
        for (auto it = dados.cbegin(); it != dados.cend(); ++it)
            linhas << QStringList{it.key(), QString::number(it.value())};
        nomeArquivo = "relatorio_top_produtos.csv";
        break;
    }
    case 3: { // Formas de pagamento
        auto dados = relatoriosServ.buscarFormasPagamentoPeriodo(
            ui->DE_InicioFormas->date(), ui->DE_FimFormas->date(),
            agrupFromCombo(ui->CBox_AgrupFormas, /*semDia=*/true));

        QSet<QString> periodSet;
        for (auto &inner : dados)
            for (auto it = inner.cbegin(); it != inner.cend(); ++it)
                periodSet.insert(it.key());
        QStringList periodos = periodSet.values();
        periodos.sort();
        QStringList formas = dados.keys();

        QStringList cabecalho = {"Período"};
        cabecalho << formas;
        linhas << cabecalho;

        for (const QString &p : periodos) {
            QStringList linha = {p};
            for (const QString &f : formas)
                linha << QString::number(dados.value(f).value(p, 0));
            linhas << linha;
        }
        nomeArquivo = "relatorio_formas_pagamento.csv";
        break;
    }
    case 4: { // NF valores
        auto dados = relatoriosServ.buscarValoresNfPeriodo(
            ui->DE_InicioNF->date(), ui->DE_FimNF->date(),
            agrupFromCombo(ui->CBox_AgrupNF, /*semDia=*/true),
            configDTO.tpAmbFiscal);
        linhas << QStringList{"Período", "Valor Total (R$)"};
        for (auto it = dados.cbegin(); it != dados.cend(); ++it)
            linhas << QStringList{it.key(), QString::number(it.value(), 'f', 2)};
        nomeArquivo = "relatorio_nf_valores.csv";
        break;
    }
    case 5: { // Produtos mais lucrativos
        auto dados = relatoriosServ.produtosMaisLucrativosPeriodo(
            ui->DE_InicioLucro->date(), ui->DE_FimLucro->date());
        linhas << QStringList{"Produto", "Lucro (R$)"};
        for (auto it = dados.cbegin(); it != dados.cend(); ++it)
            linhas << QStringList{it.key(), QString::number(it.value(), 'f', 2)};
        nomeArquivo = "relatorio_lucro_produtos.csv";
        break;
    }
    default:
        QMessageBox::information(this, "Não disponível",
            "Esta página não possui dados para exportar.");
        return;
    }

    CsvExporter::exportar(this, nomeArquivo, linhas);
}
