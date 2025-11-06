#include "schemamanager.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QSql>
#include <QSqlError>
#include "configuracao.h"
#include "mainwindow.h"

SchemaManager::SchemaManager(QObject *parent, int dbLastVersion)
    : QObject{parent}
{
    dbSchemaLastVersion = dbLastVersion;

    QSqlDatabase::addDatabase("QSQLITE");

    db = QSqlDatabase::database();

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

    db.close();
}

void SchemaManager::update() {
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados.";
    }

    QSqlQuery query;
    // obter a versao do esquema do banco de dados
    if (query.exec("PRAGMA user_version")) {
        if (query.next()) {
            dbSchemaVersion = query.value(0).toInt();
        }
    } else {
        qDebug() << "Failed to execute PRAGMA user_version:";
    }
    query.finish();
    qDebug() << dbSchemaVersion;

    QMap<QString, QString> financeiroValues = Configuracao::get_All_Financeiro_Values();

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
                QString descricaoNormalizada = MainWindow::normalizeText(descricao);

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
                QString descricaoNormalizada = MainWindow::normalizeText(descricao);

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

            dbSchemaVersion = 4;

            break;
        }
        case 4:
        {

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

            dbSchemaVersion = 5;

            break;
        }
        case 5:
        {
            // comecar transacao
            if (!db.transaction()) {
                qDebug() << "Error: unable to start transaction";
            }

            QSqlQuery query;

            // Adicionar colunas à tabela produtos
            QStringList alterStatements = {
                "ALTER TABLE notas_fiscais ADD COLUMN cnpjemit TEXT",
                "ALTER TABLE notas_fiscais ADD COLUMN chnfe TEXT",
                "ALTER TABLE notas_fiscais ADD COLUMN nprot TEXT",
                "ALTER TABLE notas_fiscais ADD COLUMN cuf TEXT",
                "CREATE TABLE eventos_fiscais(id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT, tipo_evento TEXT,"
                "id_lote INTEGER, cstat TEXT, justificativa TEXT, codigo TEXT, xml_path TEXT,"
                "nprot TEXT, id_nf INTEGER, atualizado_em DATETIME DEFAULT CURRENT_TIMESTAMP, FOREIGN KEY "
                "(id_nf) REFERENCES notas_fiscais(id))",
                "INSERT INTO config (key, value) VALUES ('usar_ibs', '0')",
                "ALTER TABLE notas_fiscais ADD COLUMN finalidade TEXT",
                "ALTER TABLE notas_fiscais ADD COLUMN saida BOOL",
                "ALTER TABLE notas_fiscais ADD COLUMN id_nf_ref INTEGER",
                "UPDATE notas_fiscais SET saida = 1",
                "UPDATE notas_fiscais SET finalidade = 'NORMAL'"
            };

            foreach (const QString &sql, alterStatements) {
                if (!query.exec(sql)) {
                    qDebug() << "Erro ao executar:" << sql << ":" << query.lastError().text();
                }
            }

            // Atualizar versão do schema se tudo correu bem
            if (!query.exec("PRAGMA user_version = 6")) {
                qDebug() << "Erro ao atualizar user_version:" << query.lastError().text();
            }

            // Finalizar transação
            if (!db.commit()) {
                qDebug() << "Error: unable to commit transaction";
                db.rollback();
            } else {
                dbSchemaVersion = 6;
                emit dbVersao6();

            }

            break;
        }

        }
    }
    qDebug() << dbSchemaVersion;
    qDebug() << db.tables();

    db.close();
}
