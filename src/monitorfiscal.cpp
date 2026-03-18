#include "monitorfiscal.h"
#include "ui_monitorfiscal.h"
#include "nota/DanfeUtil.h"
#include "delegatehora.h"
#include "delegateambiente.h"

MonitorFiscal::MonitorFiscal(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MonitorFiscal)
{
    ui->setupUi(this);

    m_tipoAtual = TipoVisualizacao::NotaFiscal;

    ui->frame->setMinimumWidth(60);
    ui->frame->setMaximumWidth(150);
    ui->frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    modelSaida = new QSqlQueryModel(this);
    modelEventos = new QSqlQueryModel(this);
    ui->TView_Fiscal->setModel(modelSaida);


    auto item1 = new MenuItem("Saída");
    auto item2 = new MenuItem("Devolução");
    auto item3 = new MenuItem("Eventos");
    auto item4 = new MenuItem("Entrada");

    ui->verticalLayout->addWidget(item1);
    ui->verticalLayout->addWidget(item2);
    ui->verticalLayout->addWidget(item3);
    ui->verticalLayout->addWidget(item4);

    m_items = { item1, item2, item3, item4 };

    for (auto item : m_items) {
        connect(item->button(), &QPushButton::clicked, this, [=]() {
            selectItem(item);
        });
    }
    delegateHora = new DelegateHora(ui->TView_Fiscal);
    delegateAmb = new DelegateAmbiente(ui->TView_Fiscal);
    selectItem(item1);

}


void MonitorFiscal::selectItem(MenuItem* item)
{
    for (auto other : m_items) {
        if (other == item)
            other->open();
        else
            other->close();
    }

    // Ação por botão
    if (item->button()->text() == "Saída"){
        m_tipoAtual = TipoVisualizacao::NotaFiscal;
        abrirSaida();
    }
    else if (item->button()->text() == "Devolução"){
        m_tipoAtual = TipoVisualizacao::NotaFiscal;
        abrirDevolucao();
    }
    else if (item->button()->text() == "Eventos"){
        m_tipoAtual = TipoVisualizacao::Evento;
        abrirEventos();
    }
    else if (item->button()->text() == "Entrada"){
        m_tipoAtual = TipoVisualizacao::NotaFiscal;
        abrirEntrada();
    }
}

void MonitorFiscal::abrirDevolucao(){
    AtualizarTabelaNotas({"DEVOLUCAO"});
}

void MonitorFiscal::abrirEntrada(){
    AtualizarTabelaNotas({"ENTRADA EXTERNA", "resNFe"});
}

void MonitorFiscal::abrirSaida(){
    AtualizarTabelaNotas({"NORMAL"});
}

void MonitorFiscal::abrirEventos(){
    AtualizarTabelaEventos();
}


MonitorFiscal::~MonitorFiscal()
{
    delete ui;
}

void MonitorFiscal::AtualizarTabelaNotas(const QStringList &finalidades){
    ui->TView_Fiscal->setModel(modelSaida);

    notaServ.listarMonitor(modelSaida, finalidades);

    ui->TView_Fiscal->setItemDelegateForColumn(4, delegateHora);
    ui->TView_Fiscal->setItemDelegateForColumn(5, delegateAmb);

    modelSaida->setHeaderData(0, Qt::Horizontal, "ID");
    modelSaida->setHeaderData(1, Qt::Horizontal, "Valor");
    modelSaida->setHeaderData(2, Qt::Horizontal, "Modelo");
    modelSaida->setHeaderData(3, Qt::Horizontal, "Número");
    modelSaida->setHeaderData(4, Qt::Horizontal, "Data Emissão");
    modelSaida->setHeaderData(5, Qt::Horizontal, "Ambiente");
    modelSaida->setHeaderData(6, Qt::Horizontal, "Chave");
    modelSaida->setHeaderData(7, Qt::Horizontal, "CNPJ Emitente");
    modelSaida->setHeaderData(8, Qt::Horizontal, "Finalidade");
    modelSaida->setHeaderData(9, Qt::Horizontal, "Caminho XML");

    ui->TView_Fiscal->setColumnHidden(0, true);
    ui->TView_Fiscal->setColumnHidden(9, true);

    ui->TView_Fiscal->selectRow(0);
}

void MonitorFiscal::AtualizarTabelaEventos(){
    ui->TView_Fiscal->setItemDelegateForColumn(5, nullptr);
    ui->TView_Fiscal->setModel(modelEventos);

    eventoServ.listarTodos(modelEventos);

    modelEventos->setHeaderData(0, Qt::Horizontal, "ID");
    modelEventos->setHeaderData(1, Qt::Horizontal, "Tipo");
    modelEventos->setHeaderData(2, Qt::Horizontal, "CStat");
    modelEventos->setHeaderData(3, Qt::Horizontal, "Código");
    modelEventos->setHeaderData(4, Qt::Horizontal, "Data");
    modelEventos->setHeaderData(5, Qt::Horizontal, "Caminho XML");

    ui->TView_Fiscal->setColumnHidden(0, true);
    ui->TView_Fiscal->setColumnHidden(5, true);

    ui->TView_Fiscal->setColumnWidth(1, 150);
    ui->TView_Fiscal->selectRow(0);
}

void MonitorFiscal::on_Btn_Danfe_clicked()
{
    auto selectionModel = ui->TView_Fiscal->selectionModel();

    if (!selectionModel || !selectionModel->hasSelection()) {
        qDebug() << "Nenhuma linha selecionada";
        return;
    }

    int row = selectionModel->currentIndex().row();

    int colunaXml = (m_tipoAtual == TipoVisualizacao::Evento) ? 5 : 9;

    QAbstractItemModel *modelAtual =
        (m_tipoAtual == TipoVisualizacao::Evento) ? modelEventos : modelSaida;

    QModelIndex xmlIndex = modelAtual->index(row, colunaXml);
    QString xmlPath = modelAtual->data(xmlIndex).toString();

    if (xmlPath.isEmpty()) {
        qDebug() << "XML vazio";
        return;
    }

    DanfeUtil danfe;

    if (m_tipoAtual == TipoVisualizacao::Evento) {
        danfe.abrirDanfePorXmlEvento(xmlPath);
    } else {
        danfe.abrirDanfePorXml(xmlPath);
    }
}

