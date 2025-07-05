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
#include "subclass/produtotableview.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include "delegateprecovalidate.h"
#include "customdelegate.h"
#include "delegatelockcol.h"
#include "delegatequant.h"
#include <qtrpt.h>
#include <QFile>
#include <QSqlQuery>
#include <QCompleter>
#include "inserircliente.h"
#include <QStringListModel>
#include <QTimer>
#include <QSqlError>
#include <QSqlRecord>
#include <QSql>
#include "configuracao.h"
//#include <QDebug>;

relatorios::relatorios(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::relatorios)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(1);
    ui->Stacked_Estoque->setCurrentIndex(3);
    fiscalValues = Configuracao::get_All_Fiscal_Values();

    connect(ui->CBox_EstoqueMain, QOverload<int>::of(&QComboBox::currentIndexChanged),
            ui->Stacked_Estoque, &QStackedWidget::setCurrentIndex);

    configurarOrcamentoEstoque();

}


relatorios::~relatorios()
{
    delete ui;
}
void relatorios::deletarProd(){
    modeloSelecionados->removeRow(ui->Tview_ProdutosSelec->currentIndex().row());
    ui->Lbl_TotalGeral->setText(totalGeral());
}
bool relatorios::verificarNomeIdCliente(const QString &nome, int id) {
    if (!db.open()) {
        qDebug() << "Erro ao conectar ao banco de dados";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT nome FROM clientes WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erro na consulta:";
        db.close();
        return false;
    }

    if (query.next()) {
        QString nomeNoBanco = query.value(0).toString();
        db.close();
        return nomeNoBanco.compare(nome, Qt::CaseInsensitive) == 0;
    }

    db.close();
    return false;
}
QPair<QString, int> relatorios::extrairNomeId(const QString &texto) {
    QRegularExpression regex("^(.*?)\\s*\\(ID:\\s*(\\d+)\\)$");
    QRegularExpressionMatch match = regex.match(texto);

    if (match.hasMatch()) {
        return qMakePair(match.captured(1).trimmed(), match.captured(2).toInt());
    }
    return qMakePair(QString(), -1); // Retorno inválido
}
int relatorios::validarCliente(bool mostrarMensagens) {
    QString texto = ui->Ledit_Cliente->text().trimmed();

    // Se o campo estiver vazio
    if (texto.isEmpty()) {
        if (mostrarMensagens) {
            QMessageBox::warning(this, "Cliente não informado", "Por favor, informe o cliente!");
            ui->Ledit_Cliente->setFocus();
        }
        return -1; // Código de erro para campo vazio
    }

    // Extrai nome e ID do texto digitado
    auto [nome, id] = extrairNomeId(texto);

    // Verifica formato válido
    if (id == -1) {
        // Tenta encontrar o cliente mais próximo no banco de dados
        if (!db.open()) {
            if (mostrarMensagens) {
                qDebug() << "Erro ao abrir banco de dados";
                QMessageBox::warning(this, "Erro", "Não foi possível validar o cliente!");
            }
            return -2; // Código de erro para falha no banco de dados
        }

        QSqlQuery query;
        query.prepare("SELECT id, nome FROM clientes WHERE nome LIKE :nome ORDER BY LENGTH(nome) ASC LIMIT 1");
        query.bindValue(":nome", "%" + texto + "%");

        if (query.exec() && query.next()) {
            int foundId = query.value(0).toInt();
            QString foundName = query.value(1).toString();
            ui->Ledit_Cliente->setText(QString("%1 (ID: %2)").arg(foundName).arg(foundId));
            db.close();
            return foundId; // Retorna o ID encontrado
        } else {
            db.close();
            if (mostrarMensagens) {
                QMessageBox::warning(this, "Cliente não encontrado",
                                     "Nenhum cliente correspondente foi encontrado.\n"
                                     "Digite no formato: Nome (ID: 123) ou selecione uma sugestão.");
                ui->Ledit_Cliente->clear();
                ui->Ledit_Cliente->setFocus();
            }
            return -3; // Código de erro para cliente não encontrado
        }
    }

    // Verifica correspondência entre nome e ID
    if (!verificarNomeIdCliente(nome, id)) {
        if (mostrarMensagens) {
            QMessageBox::warning(this, "Dados inválidos",
                                 "O nome não corresponde ao ID informado!\n"
                                 "Por favor, corrija ou selecione uma sugestão válida.");
            ui->Ledit_Cliente->selectAll();
            ui->Ledit_Cliente->setFocus();
        }
        return -4; // Código de erro para nome e ID não correspondentes
    }

    return id; // Retorna o ID válido
}
void relatorios::atualizarListaCliente(){
    clientesComId.clear(); // Limpa a lista antes de recarregar

    if (!db.open()) {
        qDebug() << "Erro ao conectar ao banco de dados para autocompletar nomes.";
    } else {
        // Consultar nomes e IDs da tabela "clientes"
        QSqlQuery query("SELECT id, nome FROM clientes");

        while (query.next()) {
            int id = query.value(0).toInt();
            QString nome = query.value(1).toString();
            // Formatar como "Nome (ID: 123)"
            clientesComId << QString("%1 (ID: %2)").arg(nome).arg(id);
        }
        //db.close();
    }

    // Atualizar o completer
    QCompleter *completer = ui->Ledit_Cliente->completer();
    if (completer) {
        QStringListModel *model = qobject_cast<QStringListModel*>(completer->model());
        if (model) {
            model->setStringList(clientesComId);
        }
    }
}
void relatorios::configurarOrcamentoEstoque(){
    modeloSelecionados->setHorizontalHeaderItem(0, new QStandardItem("ID Produto"));
    modeloSelecionados->setHorizontalHeaderItem(1, new QStandardItem("Quantidade Vendida"));
    modeloSelecionados->setHorizontalHeaderItem(2, new QStandardItem("Descrição"));
    modeloSelecionados->setHorizontalHeaderItem(3, new QStandardItem("Preço Unitário Vendido"));
    modeloSelecionados->setHorizontalHeaderItem(4, new QStandardItem("Total"));

    ui->Tview_ProdutosSelec->setModel(modeloSelecionados);
    ui->Tview_ProdutosSelec->setColumnWidth(0, 70);
    ui->Tview_ProdutosSelec->setColumnWidth(1, 130);
    ui->Tview_ProdutosSelec->setColumnWidth(2, 300);
    ui->Tview_ProdutosSelec->setColumnWidth(3, 150);


    // -- delegates
    DelegatePrecoValidate *validatePreco = new DelegatePrecoValidate(this);
    ui->Tview_ProdutosSelec->setItemDelegateForColumn(3,validatePreco);
    DelegateLockCol *delegateLockCol = new DelegateLockCol(0,this);
    ui->Tview_ProdutosSelec->setItemDelegateForColumn(0,delegateLockCol);
    DelegateLockCol *delegateLockCol2 = new DelegateLockCol(2,this);
    DelegateLockCol *delegateLockCol3 = new DelegateLockCol(4,this);
    ui->Tview_ProdutosSelec->setItemDelegateForColumn(4,delegateLockCol3);

    ui->Tview_ProdutosSelec->setItemDelegateForColumn(2,delegateLockCol2);
    DelegateQuant *delegateQuant = new DelegateQuant(this);
    ui->Tview_ProdutosSelec->setItemDelegateForColumn(1,delegateQuant);

    connect(modeloSelecionados, &QStandardItemModel::itemChanged, this, [=]() {
        ui->Lbl_TotalGeral->setText(totalGeral());
        atualizarTotalProduto();
    });

    QCompleter *completer = new QCompleter(this);
    completer->setCaseSensitivity(Qt::CaseInsensitive); // Ignorar maiúsculas e minúsculas
    completer->setFilterMode(Qt::MatchContains); // Sugestões que contêm o texto digitado

    atualizarListaCliente();

    if (!clientesComId.isEmpty()) {
        // Define o primeiro item da lista como texto do QLineEdit
        ui->Ledit_Cliente->setText(clientesComId.first());

        // Opcional: selecionar apenas o nome (se quiser destacar parte do texto)
        // Isso depende do formato que você está usando ("Nome (ID: 123)")
        QString primeiroCliente = clientesComId.first();
        int posInicioNome = 0;
        int posFinalNome = primeiroCliente.indexOf(" (ID:"); // Encontra onde começa o ID

        if (posFinalNome != -1) {
            // Seleciona apenas o nome (sem o ID)
            ui->Ledit_Cliente->setSelection(posInicioNome, posFinalNome);
        }
    }

    QStringListModel *model = new QStringListModel(clientesComId, this);
    completer->setModel(model);
    ui->Ledit_Cliente->setCompleter(completer);


    connect(ui->Ledit_Cliente, &QLineEdit::textEdited, this, [=]() {
        completer->complete();
    });
    connect(ui->Ledit_Cliente, &QLineEdit::cursorPositionChanged, this, [=]() {
        completer->complete();
    });

    connect(ui->Ledit_Cliente, &QLineEdit::editingFinished, this, [=]() {
        validarCliente(true); // Mostra mensagens para o usuário
    });
    actionMenuDeletarProd = new QAction(this);
    actionMenuDeletarProd->setText("Deletar Produto");
    connect(actionMenuDeletarProd,SIGNAL(triggered(bool)),this,SLOT(deletarProd()));


    ui->Tview_ProdutosSelec->setContextMenuPolicy(Qt::CustomContextMenu);
    // connect(ui->Tview_ProdutosSelec, &QTableView::customContextMenuRequested,
    //         this, &relatorios::mostrarMenuContexto);

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
void relatorios::atualizarTotalProduto() {
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        double totalproduto = 0.0;

        float quantidade = portugues.toFloat(
            modeloSelecionados->data(modeloSelecionados->index(row, 1)).toString()
            ); // Coluna de quantidade

        double preco = portugues.toDouble(
            modeloSelecionados->data(modeloSelecionados->index(row, 3)).toString()
            ); // Coluna de preço

        totalproduto = quantidade * preco;

        // Atualiza o valor na coluna 4
        modeloSelecionados->setData(
            modeloSelecionados->index(row, 4),
            QString::number(totalproduto, 'f', 2) // 2 casas decimais
            );
    }
}
QString relatorios::totalGeral(){
    // Obtendo os dados da tabela e calculando o valor total da venda
    double totalValue = 0.0;
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row) {
        float quantidade = portugues.toFloat(modeloSelecionados->data(modeloSelecionados->index(row, 1)).toString());  // Coluna de quantidade
        double preco = portugues.toDouble(modeloSelecionados->data(modeloSelecionados->index(row, 3)).toString());  // Coluna de preço
        totalValue += quantidade * preco;
    }
    // total notacao br
    return portugues.toString(totalValue, 'f', 2);
}


void relatorios::on_Btn_AddProd_clicked()
{
    ProdutoTableView* ptv = qobject_cast<ProdutoTableView*>(ui->Tview_ProdutosOrcamento);
    QSqlQueryModel* modelo = ptv->getModel();

    QItemSelectionModel *selectionModel = ui->Tview_ProdutosOrcamento->selectionModel();

    // Verifica se há alguma linha selecionada
    if (!selectionModel || selectionModel->selectedIndexes().isEmpty()) {
        QMessageBox::warning(this, "Erro", "Nenhum produto selecionado!");
        return;
    }

    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = modelo->data(modelo->index(selectedIndex.row(), 0));
    QVariant descVariant = modelo->data(modelo->index(selectedIndex.row(), 2));
    QVariant precoVariant = modelo->data(modelo->index(selectedIndex.row(), 3));

    QString idProduto = idVariant.toString();
    QString descProduto = descVariant.toString();

    // preco com notação BR
    float precoProduto = precoVariant.toFloat();
    QStandardItem *itemPreco = new QStandardItem();
    itemPreco->setData(precoProduto, Qt::EditRole);  // valor bruto
    itemPreco->setText(portugues.toString(precoProduto, 'f', 2)); // texto formatado

    // montar os itens da nova linha
    QStandardItem *itemQuantidade = new QStandardItem("1");

    QStandardItem *itemTotal = new QStandardItem();
    itemTotal->setData(precoProduto, Qt::EditRole);
    itemTotal->setText(portugues.toString(precoProduto, 'f', 2));


    modeloSelecionados->appendRow({
        new QStandardItem(idProduto),
        itemQuantidade,
        new QStandardItem(descProduto),
        itemPreco,
        itemTotal
    });

    // atualizar total
    ui->Lbl_TotalGeral->setText(totalGeral());
}



void relatorios::on_Ledit_PesquisaProduto_textChanged(const QString &arg1)
{

    QString inputText = ui->Ledit_PesquisaProduto->text();
    QString normalizadoPesquisa = MainWindow::normalizeText(inputText);

    // Dividir a string em palavras usando split por espaços em branco
    QStringList palavras = normalizadoPesquisa.split(" ", Qt::SkipEmptyParts);

    // Exibir as palavras separadas no console (opcional)
    // qDebug() << "Palavras separadas:";
    // for (const QString& palavra : palavras) {
    //     qDebug() << palavra;
    // }

    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados. Botão Pesquisar.";
        return;
    }



    // Construir consulta SQL dinâmica
    QString sql = "SELECT * FROM produtos WHERE ";
    QStringList conditions;
    if (palavras.length() > 1){
        for (const QString &palavra : palavras) {
            conditions << QString("descricao LIKE '%%1%'").arg(palavra);

        }

        sql += conditions.join(" AND ");

    }else{
        sql += "descricao LIKE '%" + normalizadoPesquisa + "%'  OR codigo_barras LIKE '%" + normalizadoPesquisa + "%'";
    }
    sql += " ORDER BY id DESC";

    // Executar a consulta
    ProdutoTableView* ptv = qobject_cast<ProdutoTableView*>(ui->Tview_ProdutosOrcamento);
    QSqlQueryModel* modelo = ptv->getModel();

    modelo->setQuery(sql, db);


    // Mostrar na tableview a consulta
    // CustomDelegate *delegate = new CustomDelegate(this);
    // ui->Tview_Produtos->setItemDelegate(delegate);
    ui->Tview_ProdutosOrcamento->setModel(modelo);

    db.close();
}


void relatorios::on_Btn_Terminar_clicked()
{
    int idCliente = validarCliente(true);
    if (idCliente < 0) { // Se retornou algum código de erro
        return;
    }

     auto [nome, id] = extrairNomeId(ui->Ledit_Cliente->text());


    // pegar os valores da tabela dos produtos selecionados
    QList<QList<QVariant>> rowDataList;
    for (int row = 0; row < modeloSelecionados->rowCount(); ++row){
        QList<QVariant> rowData;
        for (int col = 0; col < modeloSelecionados->columnCount(); col++){
            QModelIndex index = modeloSelecionados->index(row, col);
            rowData.append(modeloSelecionados->data(index));
        }
        rowDataList.append(rowData);
    }
    qDebug() << rowDataList;

    //QString cliente = ui->Ledit_Cliente->text();
    QString data = portugues.toString(QDateTime::currentDateTime(), "dd-MM-yyyy hh:mm:ss");
    if(!db.open()){
        qDebug() << "bd nao abriu botao ver orcamento";
    }

    QMap<QString, QString> dadosEmpresa;

    QSqlQuery query;
    query.exec("SELECT key, value FROM config WHERE key IN ("
               "'nome_empresa', "
               "'endereco_empresa', "
               "'telefone_empresa', "
               "'cnpj_empresa', "
               "'email_empresa', "
               "'cidade_empresa', "
               "'estado_empresa', "
               "'caminho_logo_empresa')");

    while (query.next()) {
        QString key = query.value(0).toString();
        QString value = query.value(1).toString();
        dadosEmpresa[key] = value;
    }

    QString nomeEmpresa = dadosEmpresa.value("nome_empresa", "");
    QString caminhoLogo = dadosEmpresa.value("caminho_logo_empresa", "");
    QString endereco_empresa = dadosEmpresa.value("endereco_empresa", "");
    QString cnpj_empresa = dadosEmpresa.value("cnpj_empresa", "");
    QString email_empresa = dadosEmpresa.value("email_empresa", "");
    QString cidade_empresa = dadosEmpresa.value("cidade_empresa", "");
    QString estado_empresa = dadosEmpresa.value("estado_empresa", "");
    QString telefone_empresa = dadosEmpresa.value("telefone_empresa", "");

    query.prepare("SELECT email, telefone, endereco, cpf FROM clientes where id = :id");
    query.bindValue(":id", idCliente);
    QString emailCliente,telefoneCliente,enderecoCliente,cpfCliente;
    query.exec();
    while(query.next()){
        emailCliente = query.value(0).toString();
        telefoneCliente = query.value(1).toString();
        enderecoCliente = query.value(2).toString();
        cpfCliente = query.value(3).toString();
    }
    db.close();

    QString observacao = ui->Tedit_Obs->toPlainText();

    QtRPT *report = new QtRPT(this);
    qDebug() << QCoreApplication::applicationDirPath();
    report->loadReport(QCoreApplication::applicationDirPath() + "/reports/orcamentoReport.xml");


    connect(report, &QtRPT::setDSInfo, [&](DataSetInfo &dsinfo){
        dsinfo.recordCount = rowDataList.size();
    });
    connect(report, &QtRPT::setValue, [&](const int recno, const QString paramname, QVariant &paramvalue, const int reportpage) {
        Q_UNUSED(reportpage);

        if (paramname == "nomeEmpresa") {
            paramvalue = nomeEmpresa;
        }else if( paramname == "endereco"){
            paramvalue = endereco_empresa;
        }else if(paramname == "cidade"){
            paramvalue = cidade_empresa;
        }else if(paramname == "estado"){
            paramvalue = estado_empresa;
        }else if(paramname == "email"){
            paramvalue = email_empresa;
        }else if(paramname == "cnpj"){
            paramvalue = cnpj_empresa;
        }else if(paramname == "telefone"){
            paramvalue = telefone_empresa;
        }else if(paramname == "obs"){
            paramvalue = observacao;
        }else if(paramname == "total_geral"){
            paramvalue = totalGeral();
        }else if(paramname == "nome_cliente"){
            paramvalue = nome;
        }else if(paramname == "endereco_cliente"){
            paramvalue = enderecoCliente;
        }else if(paramname == "cpf_cliente"){
            paramvalue = cpfCliente;
        }else if(paramname == "email_cliente"){
            paramvalue = emailCliente;
        }else if(paramname == "telefone_cliente"){
            paramvalue = telefoneCliente;
        }else if(paramname == "data"){
            paramvalue = data;
        }

        // Para campos que representam dados da lista
        if (recno < rowDataList.size()) {
            auto rowData = rowDataList.at(recno);


            if (paramname == "id_produto") {
                paramvalue = rowData.at(0); // Código
            } else if (paramname == "nome_produto") {
                paramvalue = rowData.at(2); // Descrição
            } else if (paramname == "quantidade") {
                paramvalue = rowData.at(1); // Quantidade
            } else if (paramname == "preco_unitario") {
                paramvalue = rowData.at(3); // Valor Unitário
            } else if (paramname == "subtotal") {
                paramvalue = rowData.at(4); // Valor Total
            }
        }
    });
    connect(report, &QtRPT::setValueImage, [&](const int recno, const QString paramname, QImage &paramvalue, const int reportpage){
        Q_UNUSED(reportpage);
        Q_UNUSED(recno);

        if(paramname == "imgLogo"){


            auto *img = new QImage(QCoreApplication::applicationDirPath() + "/" + caminhoLogo);
            //qDebug() << QCoreApplication::applicationDirPath() + "/" + caminhoLogo;
            paramvalue = *img;


        }
    });
    report->printExec();

}
void relatorios::selecionarClienteNovo(){
    atualizarListaCliente();
    if (!clientesComId.isEmpty()) {
        // Define o ultimo item da lista como texto do QLineEdit

        ui->Ledit_Cliente->setText(clientesComId.last());

        // Opcional: selecionar apenas o nome (se quiser destacar parte do texto)
        // Isso depende do formato que você está usando ("Nome (ID: 123)")
        QString ultimoCliente = clientesComId.last();
        int posInicioNome = 0;
        int posFinalNome = ultimoCliente.indexOf(" (ID:"); // Encontra onde começa o ID

        if (posFinalNome != -1) {
            // Seleciona apenas o nome (sem o ID)
            ui->Ledit_Cliente->setSelection(posInicioNome, posFinalNome);
        }
    }

}




void relatorios::on_Btn_NovoCliente_clicked()
{
    InserirCliente *inserirCliente = new InserirCliente;
    inserirCliente->setWindowModality(Qt::ApplicationModal);
    connect(inserirCliente, &InserirCliente::clienteInserido, this, &relatorios::selecionarClienteNovo);
    inserirCliente->show();
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


void relatorios::on_tabWidget_tabBarClicked(int index)
{
    if(index == 0){
        if(existeProdutoVendido()){
            db.open();
            ui->Stacked_Vendas->setCurrentIndex(3);
            ui->CBox_VendasMain->setCurrentIndex(3);
            ui->tabWidget->setCurrentIndex(0);
            connect(ui->CBox_VendasMain, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    ui->Stacked_Vendas, &QStackedWidget::setCurrentIndex);
            configurarJanelaQuantVendas();
            configurarJanelaValorVendas();
            configurarJanelaTopProdutosVendas();
            configurarJanelaFormasPagamentoAno();
            configurarJanelaNFValor();

        }else{
            QMessageBox::warning(this, "Acesso contido", "você deve vender algum produto antes de "
                                                                "visualizar os relatórios de vendas!");
            QTimer::singleShot(0, [this]() {
                ui->tabWidget->setCurrentIndex(1); // ou qualquer aba segura que você queira mostrar
            });
        }


    }
}


void relatorios::on_Tview_ProdutosSelec_customContextMenuRequested(const QPoint &pos)
{
    if(!ui->Tview_ProdutosSelec->currentIndex().isValid())
        return;
    QMenu menu(this);

    menu.addAction(actionMenuDeletarProd);


    menu.exec(ui->Tview_ProdutosSelec->viewport()->mapToGlobal(pos));
}
QMap<QString, float> relatorios::buscarValoresNfAno(const QString &ano) {

    QMap<QString, float> valores;
    QString tpamb = fiscalValues.value("tp_amb");
    qDebug() << ano;
    QSqlQuery query;
    query.prepare("SELECT strftime('%m', atualizado_em) AS mes, SUM(valor_total) "
                  "FROM notas_fiscais "
                  "WHERE (strftime('%Y', atualizado_em) = :ano "
                  "AND (cstat = '100' OR cstat = '150')) AND tp_amb = :tpamb "
                  "GROUP BY mes");
    query.bindValue(":ano", ano);
    query.bindValue(":tpamb", tpamb);

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
                QMessageBox::information(this, "Sem dados", "Não há vendas registradas para esse ano.");
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

