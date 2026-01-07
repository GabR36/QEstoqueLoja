#include "monitorfiscal.h"
#include "ui_monitorfiscal.h"
#include "nota/DanfeUtil.h"

MonitorFiscal::MonitorFiscal(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MonitorFiscal)
{
    ui->setupUi(this);

    db = QSqlDatabase::database();

    ui->frame->setMinimumWidth(60);
    ui->frame->setMaximumWidth(150);
    ui->frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    modelSaida = new QSqlQueryModel(this);
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
    if (item->button()->text() == "Saída")
        abrirSaida();
    else if (item->button()->text() == "Devolução"){
        abrirDevolucao();
    }
    else if (item->button()->text() == "Eventos"){
        abrirEventos();
    }
    else if (item->button()->text() == "Entrada"){
        abrirEntrada();
    }
}

void MonitorFiscal::abrirDevolucao(){
    AtualizarTabelaNotas("WHERE finalidade = 'DEVOLUCAO'");
}

void MonitorFiscal::abrirEntrada(){
    AtualizarTabelaNotas("WHERE finalidade = 'ENTRADA EXTERNA'");
}

void MonitorFiscal::abrirSaida(){
    AtualizarTabelaNotas("WHERE finalidade = 'NORMAL'");
}

void MonitorFiscal::abrirEventos(){
    AtualizarTabelaEventos("WHERE cstat != ''");
}

void MonitorFiscal::carregarTabelaSaida()
{
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Erro ao abrir banco carregarTabela()";
        }
    }

    modelSaida->setQuery(R"(
    SELECT
        id,
        valor_total,
        modelo,
        nnf,
        dhemi,
        tp_amb,
        chnfe,
        cnpjemit,
        finalidade,
        xml_path
    FROM notas_fiscais
    WHERE finalidade = 'NORMAL'
    ORDER BY dhemi DESC
)");
    modelSaida->setHeaderData(0, Qt::Horizontal, "ID");
    modelSaida->setHeaderData(1, Qt::Horizontal, "Valor");
    modelSaida->setHeaderData(2, Qt::Horizontal, "Modelo");
    modelSaida->setHeaderData(3, Qt::Horizontal, "Número");
    modelSaida->setHeaderData(4, Qt::Horizontal, "Data Emissão");
    modelSaida->setHeaderData(5, Qt::Horizontal, "Modelo");
    modelSaida->setHeaderData(6, Qt::Horizontal, "Ambiente");
    modelSaida->setHeaderData(7, Qt::Horizontal, "Tipo");

    if (modelSaida->lastError().isValid()) {
        qDebug() << "Erro ao carregar tabela:" << modelSaida->lastError();
    }

    ui->TView_Fiscal->setColumnHidden(0, true); // Oculta id

    ui->TView_Fiscal->setModel(modelSaida);

    db.close();
}


MonitorFiscal::~MonitorFiscal()
{
    delete ui;
}

void MonitorFiscal::AtualizarTabelaNotas(QString whereSql){
        if (!db.isOpen()) {
            if (!db.open()) {
                qDebug() << "Erro ao abrir banco atualizarTabela()";
                return;
            }
        }

        QString sql = R"(
            SELECT
                id,
                valor_total,
                modelo,
                nnf,
                dhemi,
                tp_amb,
                chnfe,
                cnpjemit,
                finalidade,
                xml_path
            FROM notas_fiscais )";

        if (!whereSql.isEmpty()) {
            sql += whereSql;
        }

        sql += " ORDER BY dhemi DESC";

        modelSaida->setHeaderData(0, Qt::Horizontal, "ID");
        modelSaida->setHeaderData(1, Qt::Horizontal, "Valor");
        modelSaida->setHeaderData(2, Qt::Horizontal, "Modelo");
        modelSaida->setHeaderData(3, Qt::Horizontal, "Número");
        modelSaida->setHeaderData(4, Qt::Horizontal, "Data Emissão");
        modelSaida->setHeaderData(5, Qt::Horizontal, "Modelo");
        modelSaida->setHeaderData(6, Qt::Horizontal, "Ambiente");
        modelSaida->setHeaderData(7, Qt::Horizontal, "Tipo");
        modelSaida->setHeaderData(7, Qt::Horizontal, "Caminho XML");

        ui->TView_Fiscal->setColumnHidden(0, true); // Oculta id


        modelSaida->setQuery(sql);
        ui->TView_Fiscal->selectRow(0);
}

void MonitorFiscal::AtualizarTabelaEventos(QString whereSql){
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Erro ao abrir banco atualizarTabela()";
            return;
        }
    }

    QString sql = R"(
            SELECT
                id,
                tipo_evento,
                cstat,
                codigo,
                atualizado_em,
                xml_path FROM eventos_fiscais )";

    if (!whereSql.isEmpty()) {
        sql += whereSql;
    }

    sql += " ORDER BY atualizado_em DESC";

    modelSaida->setHeaderData(0, Qt::Horizontal, "ID");
    modelSaida->setHeaderData(1, Qt::Horizontal, "Tipo");
    modelSaida->setHeaderData(2, Qt::Horizontal, "CStat");
    modelSaida->setHeaderData(3, Qt::Horizontal, "Código");
    modelSaida->setHeaderData(4, Qt::Horizontal, "Data");
    modelSaida->setHeaderData(7, Qt::Horizontal, "Caminho XML");

    ui->TView_Fiscal->setColumnHidden(0, true); // Oculta id

    modelSaida->setQuery(sql);
    ui->TView_Fiscal->selectRow(0);
}

void MonitorFiscal::on_Btn_Danfe_clicked()
{
    auto selectionModel = ui->TView_Fiscal->selectionModel();

    if (!selectionModel || !selectionModel->hasSelection()) {
        qDebug() << "Nenhuma linha selecionada";
        return;
    }

    QModelIndex index = selectionModel->currentIndex();
    if (!index.isValid()) {
        qDebug() << "Indice invalido";
        return;
    }

    int row = index.row();

    QModelIndex xmlIndex = modelSaida->index(row, 9); // coluna xml_path
    QString xmlPath = modelSaida->data(xmlIndex).toString();

    if (xmlPath.isEmpty()) {
        qDebug() << "XML vazio para a linha selecionada";
        return;
    }

    DanfeUtil danfe;
    danfe.abrirDanfePorXml(xmlPath);
}

