#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>
#include "customdelegate.h"
#include "alterarproduto.h"
#include "QItemSelectionModel"
#include <qsqltablemodel.h>
#include <QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include "vendas.h"
#include <QDoubleValidator>
#include "relatorios.h"
#include "venda.h"
#include <QModelIndex>
#include <QMenu>
#include <QFontDatabase>
#include <zint.h>
#include "delegateprecof2.h"
#include "util/pdfexporter.h"
#include "clientes.h"
#include "pagamentovenda.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QSql>
#include  "inserirproduto.h"
#include "subclass/leditdialog.h"
#include "infojanelaprod.h"
#include <QStandardPaths>
#include "util/helppage.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // Configura o nome do aplicativo (importante para definir a pasta no Linux)
    QCoreApplication::setApplicationName("QEstoqueLoja");

    QString dbPath;

#if defined(Q_OS_LINUX)
    // Linux: ~/.local/share/QEstoqueLoja/estoque.db
    dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/estoque.db";
    qDebug() << dbPath;
#elif defined(Q_OS_WIN)
    // Windows: %APPDATA%\QEstoqueLoja\estoque.db
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");  // Cria a pasta se não existir
    }
    dbPath = appDataPath + "/estoque.db";
#endif

    db.setDatabaseName(dbPath);
    QFileInfo dbFileInfo(dbPath);
    QDir dbDir = dbFileInfo.dir();
    if (!dbDir.exists()) {
        if (!dbDir.mkpath(".")) {
            qDebug() << "Erro ao criar diretório para o banco de dados:" << dbDir.path();
        }
    }
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados.";
    }

    // configuracao do modelo e view produtos
    ui->Tview_Produtos->setModel(model);
    model->setQuery("SELECT * FROM produtos ORDER BY id DESC");
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Quantidade"));
    model->setHeaderData(2, Qt::Horizontal, tr("Descrição"));
    model->setHeaderData(3, Qt::Horizontal, tr("Preço"));
    model->setHeaderData(4, Qt::Horizontal, tr("Código de Barras"));
    model->setHeaderData(5, Qt::Horizontal, tr("NF"));

    //teste float maior que 10000

    // QString a = "21320.3";
    // QString b = "7";
    // double all = a.toDouble() * b.toInt();

    // qDebug() << portugues.toString(all);
    // criar a versao 0 se o banco de dados estiver vazio
    QSqlQuery query;

    query.exec("CREATE TABLE produtos (id INTEGER PRIMARY KEY AUTOINCREMENT, quantidade INTEGER, descricao TEXT, preco DECIMAL(10,2), codigo_barras VARCHAR(20), nf BOOLEAN)");
    if (query.isActive()) {
        qDebug() << "Tabela produtos criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela produtos";
    }
    query.exec("CREATE TABLE vendas2 (id INTEGER PRIMARY KEY AUTOINCREMENT, cliente TEXT, data_hora DATETIME DEFAULT CURRENT_TIMESTAMP, total DECIMAL(10,2))");
    if (query.isActive()) {
        qDebug() << "Tabela de vendas2 criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela de vendas2: ";
    }
    query.exec("CREATE TABLE produtos_vendidos (id INTEGER PRIMARY KEY AUTOINCREMENT, id_produto INTEGER, id_venda INTEGER, quantidade INTEGER, preco_vendido DECIMAL(10,2), FOREIGN KEY (id_produto) REFERENCES produtos(id), FOREIGN KEY (id_venda) REFERENCES vendas2(id))");
    if (query.isActive()) {
        qDebug() << "Tabela de produtos_vendidos criada com sucesso!";
    } else {
        qDebug() << "Erro ao criar tabela de produtos_vendidos: ";
    }

    qDebug() << db.tables();

    // obter a versao do esquema do banco de dados
    int dbSchemaVersion;
    if (query.exec("PRAGMA user_version")) {
        if (query.next()) {
            dbSchemaVersion = query.value(0).toInt();
        }
    } else {
        qDebug() << "Failed to execute PRAGMA user_version:";
    }
    query.finish();
    qDebug() << dbSchemaVersion;
    financeiroValues = Configuracao::get_All_Financeiro_Values();

    // a versão mais recente do esquema do banco de dados
    int dbSchemaLastVersion = 5;

    while (dbSchemaVersion < dbSchemaLastVersion){
        // selecionar a atualizacao conforme a versao atual do banco de dados
        switch (dbSchemaVersion) {
        case 0:
        {
            // atualizar da versao 0 para a versao 1 do schema

            // comecar transacao
            if (!db.transaction()) {
                qDebug() << "Error: unable to start transaction";
            }

            QSqlQuery query;

            query.exec("ALTER TABLE vendas2 ADD COLUMN forma_pagamento VARCHAR(20)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN valor_recebido DECIMAL(10,2)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN troco DECIMAL(10,2)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN taxa DECIMAL(10,2)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN valor_final DECIMAL(10,2)");
            query.exec("ALTER TABLE vendas2 ADD COLUMN desconto DECIMAL(10,2)");

            // converter as datas no formato dd-MM-yyyy para yyyy-MM-dd
            query.exec("UPDATE vendas2 "
                       "SET data_hora = strftime('%Y-%m-%d %H:%M:%S', substr(data_hora, 7, 4) || '-' || substr(data_hora, 4, 2) || '-' || substr(data_hora, 1, 2) || ' ' || substr(data_hora, 12, 8)) "
                       "WHERE substr(data_hora, 3, 1) = '-' AND substr(data_hora, 6, 1) = '-'");

            // colocar valores nas novas colunas
            query.exec("UPDATE vendas2 SET forma_pagamento = 'Não Sei', "
                       "valor_recebido = total, "
                       "troco = 0, taxa = 0, "
                       "valor_final = total,"
                       "desconto = 0");

            // mudar a versao para 1
            query.exec("PRAGMA user_version = 1");

            query.finish();

            // terminar transacao
            if (!db.commit()) {
                qDebug() << "Error: unable to commit transaction";
                db.rollback(); // Desfaz a transação
            }

            dbSchemaVersion = 1;

            break;
        }
        case 1:
        {
            // schema versao 1 atualizar para a versao 2

            // comecar transacao
            if (!db.transaction()) {
                qDebug() << "Error: unable to start transaction";
            }
            QSqlQuery query;

            query.exec("CREATE TABLE config (id INT AUTO_INCREMENT PRIMARY KEY, "
                       "key VARCHAR(255) NOT NULL UNIQUE, "
                       "value TEXT)");
            query.exec("INSERT INTO config (key, value) VALUES ('nome_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('endereco_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('telefone_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('cnpj_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('email_empresa', '')");
            query.exec("INSERT INTO config (key, value) VALUES ('porcent_lucro', '40')");
            query.exec("INSERT INTO config (key, value) VALUES ('taxa_debito', '2')");
            query.exec("INSERT INTO config (key, value) VALUES ('taxa_credito', '3')");

            // normalizar dados existentes
            if (!query.exec("SELECT id, descricao FROM produtos")) {
                qDebug() << "Erro ao executar a consulta SQL:" << query.lastError().text();
            }

            // Iterar sobre os resultados da consulta
            while (query.next()) {
                int id = query.value(0).toInt();
                QString descricao = query.value(1).toString();

                // Normalizar a descrição
                QString descricaoNormalizada = normalizeText(descricao);

                // Atualizar a tabela produtos com a descrição normalizada
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE produtos SET descricao = :descricao WHERE id = :id");
                updateQuery.bindValue(":descricao", descricaoNormalizada);
                updateQuery.bindValue(":id", id);

                if (!updateQuery.exec()) {
                    qDebug() << "Erro ao atualizar a descrição do produto:" << updateQuery.lastError().text();
                    db.rollback();
                }
            }

            // mudar a versao para 2
            query.exec("PRAGMA user_version = 2");

            // terminar transacao
            if (!db.commit()) {
                qDebug() << "Error: unable to commit transaction";
                db.rollback(); // Desfaz a transação
            }

            dbSchemaVersion = 2;

            break;
        }
        case 2:
        {
            // schema versao 2 atualizar para a versao 3

            // comecar transacao
            if (!db.transaction()) {
                qDebug() << "Error: unable to start transaction";
            }
            QSqlQuery query;

            query.exec("CREATE TABLE entradas_vendas (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "id_venda INTEGER, "
                       "total DECIMAL(10,2),"
                       "data_hora DATETIME DEFAULT CURRENT_TIMESTAMP,"
                       "forma_pagamento VARCHAR(20),"
                       "valor_recebido DECIMAL(10,2),"
                       "troco DECIMAL(10,2),"
                       "taxa DECIMAL(10,2),"
                       "valor_final DECIMAL(10,2),"
                       "desconto DECIMAL(10,2),"
                       "FOREIGN KEY (id_venda) REFERENCES vendas2(id))");

            // normalizar dados existentes
            if (!query.exec("SELECT id, descricao FROM produtos")) {
                qDebug() << "Erro ao executar a consulta SQL:" << query.lastError().text();
            }
            // Iterar sobre os resultados da consulta
            while (query.next()) {
                int id = query.value(0).toInt();
                QString descricao = query.value(1).toString();

                // Normalizar a descrição
                QString descricaoNormalizada = normalizeText(descricao);

                // Atualizar a tabela produtos com a descrição normalizada
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE produtos SET descricao = :descricao WHERE id = :id");
                updateQuery.bindValue(":descricao", descricaoNormalizada);
                updateQuery.bindValue(":id", id);

                if (!updateQuery.exec()) {
                    qDebug() << "Erro ao atualizar a descrição do produto:" << updateQuery.lastError().text();
                    db.rollback();
                }
            }
            if(!query.exec("ALTER TABLE vendas2 ADD COLUMN esta_pago BOOLEAN DEFAULT 1")){
                qDebug() << "erro ao adicionar coluna estapago";
            }

            if (!query.exec("SELECT id FROM vendas2")) {
                qDebug() << "Erro ao executar a consulta SQL:" << query.lastError().text();
            }
            // Iterar sobre os resultados da consulta
            while (query.next()) {
                int id = query.value(0).toInt();
                // bool esta_pago = query.value(1).toInt();

                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE vendas2 SET esta_pago = 1 WHERE id = :id");
                updateQuery.bindValue(":id", id);

                if (!updateQuery.exec()) {
                    qDebug() << "Erro ao atualizar estapago do venda:" << updateQuery.lastError().text();
                    db.rollback();
                }
            }

            // mudar a versao para 3
            query.exec("PRAGMA user_version = 3");

            // terminar transacao
            if (!db.commit()) {
                qDebug() << "Error: unable to commit transaction";
                db.rollback(); // Desfaz a transação
            }

            dbSchemaVersion = 3;

            break;
        }
        case 3:
        {
            // schema versao 3 atualizar para a versao 4

            db.open();

            // comecar transacao
            if (!db.transaction()) {
                qDebug() << "Error: unable to start transaction";
            }

            QSqlQuery query;

            query.exec("CREATE TABLE clientes (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "nome TEXT NOT NULL,"
                       "email TEXT,"
                       "telefone TEXT,"
                       "endereco TEXT,"
                       "cpf TEXT,"
                       "data_nascimento DATE,"
                       "data_cadastro DATETIME DEFAULT CURRENT_TIMESTAMP,"
                       "eh_pf BOOLEAN )");
            query.exec("INSERT INTO clientes(nome, eh_pf) VALUES ('Consumidor', true)");

            if(!query.exec("CREATE TABLE vendas (  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "cliente TEXT, "
                            "data_hora DATETIME DEFAULT CURRENT_TIMESTAMP,"
                            "total DECIMAL(10,2), "
                            "forma_pagamento VARCHAR(20), "
                            "valor_recebido DECIMAL(10,2),"
                            "troco DECIMAL(10,2),"
                            "taxa DECIMAL(10,2),"
                            "valor_final DECIMAL(10,2),"
                            "desconto DECIMAL(10,2),"
                            "esta_pago BOOLEAN DEFAULT 1,"
                            "id_cliente INTEGER,"
                            "FOREIGN KEY (id_cliente) REFERENCES clientes (id)) ")){
                qDebug() << "Erro ao criar nova tabela vendas";
            }

            if(!query.exec("INSERT INTO vendas (id, cliente, data_hora, total, forma_pagamento, "
                            "valor_recebido, troco, taxa, valor_final, desconto, esta_pago, id_cliente) "
                            "SELECT id, cliente, data_hora, total, forma_pagamento, "
                            "valor_recebido, troco, taxa, valor_final, desconto, esta_pago, NULL "
                            "FROM vendas2")){
                qDebug() << "nao copiou dados de vendas2 para vendas";
            }

            if (!query.exec("DROP TABLE vendas2")) {
                qDebug() << "Erro ao dropar tabela:" << query.lastError().text();
            }

            if(!query.exec("ALTER TABLE vendas RENAME TO vendas2")){
                qDebug() << "nao renomeou tabela vendas para vendas2";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('cidade_empresa', '')")){
                qDebug() << "nao inserir config cidade_empresa";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('estado_empresa', '')")){
                qDebug() << "nao inserir config estado_empresa";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('caminho_logo_empresa', '')")){
                qDebug() << "nao inserir config caminho_logo_empresa";
            }


            // mudar a versao para 4
            query.exec("PRAGMA user_version = 4");

            // terminar transacao
            if (!db.commit()) {
                qDebug() << "Error: unable to commit transaction";
                db.rollback(); // Desfaz a transação
            }

            db.close();

            dbSchemaVersion = 4;

            break;
        }
        case 4:
        {
            db.open();

            // comecar transacao
            if (!db.transaction()) {
                qDebug() << "Error: unable to start transaction";
            }

            QSqlQuery query;
            if(!query.exec("INSERT INTO config (key, value) VALUES ('nfant_empresa', '')")){
                qDebug() << "nao inserir config nfant_empresa";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('numero_empresa', '')")){
                qDebug() << "nao inserir config numero_empresa";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('bairro_empresa', '')")){
                qDebug() << "nao inserir config bairro_empresa";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('cep_empresa', '')")){
                qDebug() << "nao inserir config cep_empresa";
            }


            if(!query.exec("INSERT INTO config (key, value) VALUES ('regime_trib', '')")){
                qDebug() << "nao inserir config regime_trib";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('tp_amb', '')")){
                qDebug() << "nao inserir config tp_amb";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('id_csc', '')")){
                qDebug() << "nao inserir config id_csc";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('csc', '')")){
                qDebug() << "nao inserir config csc";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('caminho_schema', '')")){
                qDebug() << "nao inserir config caminho_schema";
            }


            if(!query.exec("INSERT INTO config (key, value) VALUES ('caminho_certac', '')")){
                qDebug() << "nao inserir config caminho_certac";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('caminho_certificado', '')")){
                qDebug() << "nao inserir config caminho_certificado";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('senha_certificado', '')")){
                qDebug() << "nao inserir config senha_certificado";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('cuf', '')")){
                qDebug() << "nao inserir config cuf";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('cmun', '')")){
                qDebug() << "nao inserir config cmun";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('iest', '')")){
                qDebug() << "nao inserir config iest";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('cnpj_rt', '')")){
                qDebug() << "nao inserir config cnpj_rt";
            }

            if(!query.exec("INSERT INTO config (key, value) VALUES ('nome_rt', '')")){
                qDebug() << "nao inserir config nome_rt";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('email_rt', '')")){
                qDebug() << "nao inserir config email_rt";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('fone_rt', '')")){
                qDebug() << "nao inserir config fone_rt";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('id_csrt', '')")){
                qDebug() << "nao inserir config id_cst";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('hash_csrt', '')")){
                qDebug() << "nao inserir config hash_csrt";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('emit_nf', '0')")){
                qDebug() << "nao inserir config emitnf";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('nnf_homolog', '0')")){
                qDebug() << "nao inserir config nnfhomo";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('nnf_prod', '0')")){
                qDebug() << "nao inserir config nnfprod";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('nnf_homolog_nfe', '0')")){
                qDebug() << "nao inserir config nnfprod";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('nnf_prod_nfe', '0')")){
                qDebug() << "nao inserir config nnfprod";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('ncm_padrao', '00000000')")){
                qDebug() << "nao inserir config ncm_prod";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('csosn_padrao', '102')")){
                qDebug() << "nao inserir config csosn_padrao";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('pis_padrao', '49')")){
                qDebug() << "nao inserir config pis_padrao";
            }
            if(!query.exec("INSERT INTO config (key, value) VALUES ('cest_padrao', '')")){
                qDebug() << "nao inserir config cest_padrao";
            }

            if (!query.exec(
                    "CREATE TABLE IF NOT EXISTS notas_fiscais ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "cstat TEXT, "
                    "nnf INTEGER NOT NULL, "
                    "serie TEXT NOT NULL, "
                    "modelo TEXT NOT NULL DEFAULT '65',"
                    "tp_amb BOOLEAN, "
                    "xml_path TEXT, "
                    "valor_total DECIMAL(10,2), "
                    "atualizado_em DATETIME DEFAULT CURRENT_TIMESTAMP, "
                    "id_venda INTEGER, "
                    "FOREIGN KEY (id_venda) REFERENCES vendas2(id)"
                    ")"
                    )) {
                qDebug() << "Erro ao criar tabela notas_fiscais:" << query.lastError().text();
            }

            // Adicionar colunas à tabela produtos
            QStringList alterStatements = {
                "ALTER TABLE produtos ADD COLUMN un_comercial TEXT",
                "ALTER TABLE produtos ADD COLUMN preco_fornecedor DECIMAL(10,2) NULL",
                "ALTER TABLE produtos ADD COLUMN porcent_lucro REAL",
                "ALTER TABLE produtos ADD COLUMN ncm VARCHAR(8) DEFAULT '00000000'",
                "ALTER TABLE produtos ADD COLUMN cest TEXT NULL",
                "ALTER TABLE produtos ADD COLUMN aliquota_imposto REAL NULL",
                "ALTER TABLE produtos ADD COLUMN csosn VARCHAR(5)",
                "ALTER TABLE produtos ADD COLUMN pis VARCHAR(5)",
                "ALTER TABLE produtos ADD COLUMN local TEXT NULL",
                "ALTER TABLE clientes ADD COLUMN numero_end VARCHAR(10)",
                "ALTER TABLE clientes ADD COLUMN bairro TEXT",
                "ALTER TABLE clientes ADD COLUMN xMun TEXT",
                "ALTER TABLE clientes ADD COLUMN cMun TEXT",
                "ALTER TABLE clientes ADD COLUMN uf VARCHAR(3)",
                "ALTER TABLE clientes ADD COLUMN cep TEXT",
                "ALTER TABLE clientes ADD COLUMN indIEDest REAL",
                "ALTER TABLE clientes ADD COLUMN ie TEXT"
            };
            bool hasErrors = false;

            foreach (const QString &sql, alterStatements) {
                if (!query.exec(sql)) {
                    qDebug() << "Erro ao executar:" << sql << ":" << query.lastError().text();
                    hasErrors = true;
                }
            }




            // Só atualiza as colunas se todas foram criadas com sucesso

            if (!hasErrors) {

                query.prepare("UPDATE produtos SET porcent_lucro = :porcent");
                query.bindValue(":porcent", financeiroValues.value("porcent_lucro"));
                if (!query.exec()) {
                    qDebug() << "Erro ao atualizar porcent_lucro:" << query.lastError().text();
                    hasErrors = true;
                }

                if (!query.exec("UPDATE produtos SET un_comercial = 'UN'")) {
                    qDebug() << "Erro ao atualizar un_comercial:" << query.lastError().text();
                    hasErrors = true;
                }
                if (!query.exec("UPDATE produtos SET csosn = '102'")) {
                    qDebug() << "Erro ao atualizar csosn:" << query.lastError().text();
                    hasErrors = true;
                }
                if (!query.exec("UPDATE produtos SET pis = '49'")) {
                    qDebug() << "Erro ao atualizar pis:" << query.lastError().text();
                    hasErrors = true;
                }
            }

            // Atualizar versão do schema se tudo correu bem
            if (!hasErrors) {
                if (!query.exec("PRAGMA user_version = 5")) {
                    qDebug() << "Erro ao atualizar user_version:" << query.lastError().text();
                    hasErrors = true;
                }
            }

            // Finalizar transação
            if (!hasErrors) {
                if (!db.commit()) {
                    qDebug() << "Error: unable to commit transaction";
                    db.rollback();
                } else {
                    dbSchemaVersion = 5;
                }
            } else {
                db.rollback();
                qDebug() << "Transação revertida devido a erros";
            }

            db.close();

            dbSchemaVersion = 5;
        }

        }
    }
    qDebug() << dbSchemaVersion;
    qDebug() << db.tables();

    // mostrar na tabela da aplicaçao a tabela do banco de dados.
    ui->Tview_Produtos->horizontalHeader()->setStyleSheet("background-color: rgb(33, 105, 149)");
    ui->Ledit_Pesquisa->installEventFilter(this);
    atualizarTableview();
    db.close();
    //
    // Selecionar a primeira linha da tabela
    QModelIndex firstIndex = model->index(0, 0);
    ui->Tview_Produtos->selectionModel()->select(firstIndex, QItemSelectionModel::Select);

    // ajustar tamanho colunas
    // coluna descricao
    ui->Tview_Produtos->setColumnWidth(2, 750);
    //preco
    ui->Tview_Produtos->setColumnWidth(3, 90);

    // coluna quantidade
    ui->Tview_Produtos->setColumnWidth(1, 85);
    ui->Tview_Produtos->setColumnWidth(4,110);

    // ações para menu de contexto tabela produtos
    actionMenuAlterarProd = new QAction(this);
    actionMenuDeletarProd = new QAction(this);
    actionSetLocalProd = new QAction(this);
    actionVerProduto = new QAction(this);
    actionSetLocalProd->setText("Adicionar Local Produto");
    actionVerProduto->setText("Ver Produto");
    connect(actionSetLocalProd,SIGNAL(triggered(bool)),this,SLOT(setLocalProd()));
    connect(actionVerProduto, SIGNAL(triggered(bool)),this,SLOT(verProd()));

    actionMenuDeletarProd->setText("Deletar Produto");
    actionMenuDeletarProd->setIcon(iconDelete);
    connect(actionMenuDeletarProd,SIGNAL(triggered(bool)),this,SLOT(on_Btn_Delete_clicked()));


    actionMenuAlterarProd->setText("Alterar Produto");
    actionMenuAlterarProd->setIcon(iconAlterarProduto);
    connect(actionMenuAlterarProd,SIGNAL(triggered(bool)),this,SLOT(on_Btn_Alterar_clicked()));

    actionMenuPrintBarCode1 = new QAction(this);
    actionMenuPrintBarCode1->setText("1 Etiqueta");
    connect(actionMenuPrintBarCode1,SIGNAL(triggered(bool)),this, SLOT(imprimirEtiqueta1()));

    actionMenuPrintBarCode3 = new QAction(this);
    actionMenuPrintBarCode3->setText("3 Etiquetas");
    connect(actionMenuPrintBarCode3,SIGNAL(triggered(bool)),this, SLOT(imprimirEtiqueta3()));
    // -- delegates --
    DelegatePrecoF2 *delegatePreco = new DelegatePrecoF2(this);
    ui->Tview_Produtos->setItemDelegateForColumn(3,delegatePreco);
    CustomDelegate *delegateVermelho = new CustomDelegate(this);
    ui->Tview_Produtos->setItemDelegateForColumn(1,delegateVermelho);

    setarIconesJanela();

    connect(ui->Tview_Produtos, &QTableView::doubleClicked,
            this, &MainWindow::verProd);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setarIconesJanela(){
    iconAlterarProduto.addFile(":/QEstoqueLOja/story-editor.svg");
    iconAddProduto.addFile(":/QEstoqueLOja/add-product.svg");
    iconBtnVenda.addFile(":/QEstoqueLOja/amarok-cart-view.svg");
    iconDelete.addFile(":/QEstoqueLOja/amarok-cart-remove.svg");
    iconPesquisa.addFile(":/QEstoqueLOja/edit-find.svg");
    iconBtnRelatorios.addFile(":/QEstoqueLOja/view-financial-account-investment-security.svg");
    iconImpressora.addFile(":/QEstoqueLOja/document-print.svg");
    iconClientes.addFile(":/QEstoqueLOja/user-others.svg");


    ui->Btn_AddProd->setIcon(iconAddProduto);
    ui->Btn_Venda->setIcon(iconBtnVenda);
    ui->Btn_Alterar->setIcon(iconAlterarProduto);
    ui->Btn_Delete->setIcon(iconDelete);
    ui->Btn_Pesquisa->setIcon(iconPesquisa);
    ui->Btn_Relatorios->setIcon(iconBtnRelatorios);
    ui->Btn_Clientes->setIcon(iconClientes);
}
void MainWindow::atualizarTableview(){
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. atualizarTableView";
    }
    model->setQuery("SELECT * FROM produtos ORDER BY id DESC");
    db.close();
}


void MainWindow::on_Btn_Delete_clicked()
{
    if(ui->Tview_Produtos->selectionModel()->isSelected(ui->Tview_Produtos->currentIndex())){
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QString productId = idVariant.toString();
    QString productDesc = descVariant.toString();

    // Cria uma mensagem de confirmação
    QMessageBox::StandardButton resposta;
    resposta = QMessageBox::question(
        nullptr,
        "Confirmação",
        "Tem certeza que deseja excluir o produto:\n\n"
        "id: " + productId + "\n"
        "Descrição: " + productDesc,
        QMessageBox::Yes | QMessageBox::No
    );
    // Verifica a resposta do usuário
    if (resposta == QMessageBox::Yes) {

        // remover registro do banco de dados
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. botao deletar.";
        }
        QSqlQuery query;

        query.prepare("DELETE FROM produtos WHERE id = :valor1");
        query.bindValue(":valor1", productId);
        if (query.exec()) {
            qDebug() << "Delete bem-sucedido!";
        } else {
            qDebug() << "Erro no Delete: ";
        }
        atualizarTableview();
        db.close();
    }
    else {
        // O usuário escolheu não deletar o produto
        qDebug() << "A exclusão do produto foi cancelada.";
    }
    }else{
        QMessageBox::warning(this,"Erro","Selecione um produto antes de tentar deletar!");
    }
}
QString MainWindow::normalizeText(const QString &text) {
    QString normalized = text.normalized(QString::NormalizationForm_D);
    QString result;
    for (const QChar &c : normalized) {
        if (!c.isMark()) {
            QChar replacement;
            switch (c.unicode()) {
            case ';':
            case '\'':
            case '\"':
                // Remover os caracteres ; ' "
                continue;
            case '<':
                replacement = '(';
                break;
            case '>':
                replacement = ')';
                break;
            case '&':
                replacement = 'e';
                break;
            default:
                result.append(c.toUpper());
                continue;
            }
            result.append(replacement);
        }
    }
    return result;



}

void MainWindow::on_Btn_Pesquisa_clicked()
{
    QString inputText = ui->Ledit_Pesquisa->text();
    QString normalizadoPesquisa = normalizeText(inputText);

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
    model->setQuery(sql, db);
    if (model->lastError().isValid()) {
        qDebug() << "Erro ao executar consulta:" << model->lastError().text();
    }


    db.close();
}




void MainWindow::on_Btn_Alterar_clicked()
{


    if(ui->Tview_Produtos->selectionModel()->isSelected(ui->Tview_Produtos->currentIndex())){
    // obter id selecionado
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant idVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 0));
    QVariant quantVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 1));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));
    QVariant barrasVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 4));
    QVariant nfVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 5));
    QVariant uCom = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 6));
    QVariant precoForn = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 7));
    QVariant porcentLucro = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 8));
    QVariant ncm = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 9));
    QVariant cest = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 10));
    QVariant aliquotaImp = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 11));
    QVariant csosn = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 12));
    QVariant pis = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 13));


    QString productId = idVariant.toString();
    QString productQuant = portugues.toString(quantVariant.toFloat());
    QString productDesc = descVariant.toString();
    QString productPreco = portugues.toString(precoVariant.toFloat());
    QString productBarras = barrasVariant.toString();
    bool productNf = nfVariant.toBool();
    QString productUCom = uCom.toString();
    QString produtoPrecoForn;
    if(precoForn.toString().trimmed().isEmpty()){
        produtoPrecoForn = "";
    }else{
        produtoPrecoForn = portugues.toString(precoForn.toFloat());
    }
    QString productPorcentLucro = portugues.toString(porcentLucro.toFloat());
    QString productNCM = ncm.toString();
    QString productCEST = cest.toString();
    QString productAliquotaImp = portugues.toString(aliquotaImp.toFloat());
    QString productPis = pis.toString();
    QString productCsosn = csosn.toString();

    qDebug() << productId;
    qDebug() << productPreco;
    // criar janela
    AlterarProduto *alterar = new AlterarProduto;
    alterar->janelaPrincipal = this;
    alterar->idAlt = productId;
    alterar->TrazerInfo(productDesc, productQuant, productPreco, productBarras, productNf, productUCom,
    produtoPrecoForn, productPorcentLucro, productNCM, productCEST, productAliquotaImp, productCsosn, productPis);
    alterar->setWindowModality(Qt::ApplicationModal);
    alterar->show();
    connect(alterar, &AlterarProduto::produtoAlterado, this,
            &MainWindow::on_Btn_Pesquisa_clicked);
    }else{
        QMessageBox::warning(this,"Erro","Selecione um produto antes de alterar!");
    }

}


void MainWindow::on_Btn_Venda_clicked()
{
    Vendas *vendas = new Vendas;
    //vendas->setWindowModality(Qt::ApplicationModal);
    connect(vendas, &Vendas::vendaConcluidaVendas, this, &MainWindow::atualizarTableview);

    vendas->show();


}



void MainWindow::on_Btn_Relatorios_clicked()
{
    relatorios *relatorios1 = new relatorios;
    relatorios1->setWindowModality(Qt::ApplicationModal);
    relatorios1->show();
}

void MainWindow::imprimirEtiqueta1(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant barrasVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 4));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));


    imprimirEtiqueta(1, barrasVariant.toString(), descVariant.toString(), portugues.toString(precoVariant.toFloat()));


}
void MainWindow::imprimirEtiqueta3(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
    QVariant barrasVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 4));
    QVariant descVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 2));
    QVariant precoVariant = ui->Tview_Produtos->model()->data(ui->Tview_Produtos->model()->index(selectedIndex.row(), 3));


    imprimirEtiqueta(3, barrasVariant.toString(), descVariant.toString(), portugues.toString(precoVariant.toFloat()));


}
void MainWindow::imprimirEtiqueta(int quant, QString codBar, QString desc, QString preco){
    if (codBar == ""){
        QMessageBox::warning(this, "Erro", "código de barras inexistente");
        return;
    }
    // criar codigo de barras -----------
    QByteArray codBarBytes = codBar.toUtf8();
    const unsigned char* data = reinterpret_cast<const unsigned char*>(codBarBytes.constData());

    struct zint_symbol *barcode = ZBarcode_Create();
    if (!barcode) {
        qDebug() << "Erro ao criar o objeto de código de barras.";
        return;
    }

    // Definir o tipo de simbologia (Code128 neste caso)
    barcode->symbology = BARCODE_CODE128;
    barcode->output_options = BOLD_TEXT;

    // Definir os dados a serem codificados
    int error = ZBarcode_Encode(barcode, (unsigned char*)data, 0);
    if (error != 0) {
        qDebug() << "Erro ao codificar os dados: " << barcode->errtxt;
        ZBarcode_Delete(barcode);

    }

    // Gerar a imagem do código de barras e salvar como arquivo PNG ou GIF
    error = ZBarcode_Buffer(barcode, 0);
    if (error != 0) {
        qDebug() << "Erro ao criar o buffer da imagem: " << barcode->errtxt;
        ZBarcode_Delete(barcode);
        return ;
    }


    ZBarcode_Print(barcode, 0);
    // Limpar o objeto de código de barras
    ZBarcode_Delete(barcode);

    qDebug() << "Código de barras gerado com sucesso e salvo como out.png/out.gif";
    QImage codimage("out.gif");
    //  impressão ---------
    QPrinter printer;

    printer.setPageSize(QPageSize(QSizeF(80, 2000), QPageSize::Millimeter));
    // printer.setCopyCount(quant);

    QPrintDialog dialog(&printer, this);
    if(dialog.exec() == QDialog::Rejected) return;

    QPainter painter;
    painter.begin(&printer);

    // QByteArray cutCommand;
    // cutCommand.append(0x1D);
    // cutCommand.append('V');

    int ypos[2] = {5, 53};
    const int espacoEntreItens = 20;

    for(int i =0; i<quant; i++){


        if(i > 0){
            for(int j = 0; j < 2; j ++){
                ypos[j] = ypos[j] + 51;
            };
        }

        QRect descRect(0,ypos[0],145,32);
        QFont fontePainter = painter.font();
        fontePainter.setPointSize(10);
        painter.setFont(fontePainter);
        painter.drawText(descRect,Qt::TextWordWrap, desc);
        fontePainter.setBold(true);
        painter.setFont(fontePainter);
        painter.drawText(0, ypos[1], "Preço: R$" + portugues.toString(portugues.toFloat(preco), 'f', 2));
        fontePainter.setBold(false);
        painter.setFont(fontePainter);

        QRect codImageRect(140,ypos[0], 108,50);
        painter.drawImage(codImageRect, codimage);
        for(int j = 0; j < 2; j++) {
            ypos[j] = ypos[j] + espacoEntreItens;
        }


    }
    painter.end();






    }






void MainWindow::on_actionGerar_Relat_rio_CSV_triggered()
{
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


void MainWindow::on_actionRealizar_Venda_triggered()
{
    venda *inserirVenda = new venda;
    //inserirVenda->setWindowModality(Qt::ApplicationModal);
    inserirVenda->show();
}


void MainWindow::on_Btn_AddProd_clicked()
{

    InserirProduto *addProdJanela = new InserirProduto;
    addProdJanela->show();
    connect(addProdJanela, &InserirProduto::codigoBarrasExistenteSignal,
            this, &MainWindow::atualizarTableviewComQuery);
    connect(addProdJanela, &InserirProduto::produtoInserido, this,
            &MainWindow::atualizarTableview);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Verificar se o evento é uma tecla pressionada no lineEdit
    if (obj == ui->Ledit_Pesquisa && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            ui->Btn_Pesquisa->click(); // Simula um clique no botão
            return true; // Evento tratado
        }
    }

    // Processar o evento padrão
    return QMainWindow::eventFilter(obj, event);
}
QString MainWindow::gerarNumero()
{
    QString number;
    do {
        number = QString("3562%1").arg(QRandomGenerator::global()->bounded(100000), 5, 10, QChar('0'));
    } while (generatedNumbers.contains(number));

    generatedNumbers.insert(number);
    // saveGeneratedNumber(number);

    return number;
}




void MainWindow::on_Tview_Produtos_customContextMenuRequested(const QPoint &pos)
{

    if(!ui->Tview_Produtos->currentIndex().isValid())
        return;

    QMenu menu(this);
    QMenu *imprimirMenu = new QMenu("Imprimir Etiqueta Código de Barra", this);

    menu.addAction(actionMenuAlterarProd);
    menu.addAction(actionMenuDeletarProd);
    menu.addAction(actionSetLocalProd);
    menu.addAction(actionVerProduto);
    imprimirMenu->setIcon(iconImpressora);
    imprimirMenu->addAction(actionMenuPrintBarCode1);
    imprimirMenu->addAction(actionMenuPrintBarCode3);
    menu.addMenu(imprimirMenu);

    menu.exec(ui->Tview_Produtos->viewport()->mapToGlobal(pos));
}




void MainWindow::on_actionTodos_Produtos_triggered()
{

    // // salva o arquivo
    QString fileName = QFileDialog::getSaveFileName(this, "Salvar PDF", QString(), "*.pdf");
     if (fileName.isEmpty())
         return;

     PDFexporter::exportarTodosProdutosParaPDF(fileName);

 }


void MainWindow::on_actionApenas_NF_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Salvar PDF", QString(), "*.pdf");
    if (fileName.isEmpty())
        return;

    PDFexporter::exportarNfProdutosParaPDF(fileName);



}


void MainWindow::on_actionConfig_triggered()
{
    Configuracao *configuracao = new Configuracao();
    configuracao->show();
}


void MainWindow::on_Ledit_Pesquisa_textChanged(const QString &arg1)
{
    ui->Btn_Pesquisa->click();
}


void MainWindow::on_Btn_Clientes_clicked()
{


    Clientes *clientes = new Clientes;
    clientes->setWindowModality(Qt::ApplicationModal);
    clientes->show();
}

void MainWindow::atualizarTableviewComQuery(QString &query){
    model->setQuery(query);
}
void MainWindow::setLocalProd(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex idIndex = ui->Tview_Produtos->model()->index(selectedRow, 0);
        QModelIndex localIndex = ui->Tview_Produtos->model()->index(selectedRow, 14);

        int id = ui->Tview_Produtos->model()->data(idIndex).toInt();
        QString local = ui->Tview_Produtos->model()->data(localIndex).toString();
        QSqlQuery query;
        QStringList sugestoes;
        if(!db.isOpen()){
            db.open();
        }
        if (query.exec("SELECT DISTINCT local FROM produtos WHERE local IS NOT NULL AND TRIM(local) != ''")) {
            while (query.next()) {
                sugestoes << query.value(0).toString();
            }
        } else {
            qDebug() << "Erro ao executar query de locais:" << query.lastError().text();
        }

        LeditDialog dialog(this);
        dialog.setWindowTitle("Inserir Texto");
        dialog.setLabelText("Informe o local do produto:");
        dialog.setLineEditText(local);
        dialog.Ledit_info->setMaxLength(60);
        dialog.setCompleterSuggestions(sugestoes);

        if (dialog.exec() == QDialog::Accepted) {
            QString novoLocal = dialog.getLineEditText();
            qDebug() << "Novo local para ID" << id << ":" << novoLocal;

            query.prepare("UPDATE produtos SET local = :novolocal WHERE id = :id ");
            query.bindValue(":novolocal", novoLocal);
            query.bindValue(":id", id);
            if(!query.exec()){
                qDebug() << "falhou ao executar query update local";
            }
            atualizarTableview();
            emit localSetado();

        }
        db.close();
    }
}
int MainWindow::getIdProdSelected(){
    QItemSelectionModel *selectionModel = ui->Tview_Produtos->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex idIndex = ui->Tview_Produtos->model()->index(selectedRow, 0);

        int id = ui->Tview_Produtos->model()->data(idIndex).toInt();
        return id;
    }
}
void MainWindow::verProd(){
    int id = getIdProdSelected();
    InfoJanelaProd *janelaProd = new InfoJanelaProd(this, id);
    janelaProd->show();
}




void MainWindow::on_actionDocumenta_o_triggered()
{
    HelpPage *page = new HelpPage();
    page->show();
}

