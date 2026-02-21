#include "cliente_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>
#include "../util/datautil.h"

Cliente_repository::Cliente_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();

}

qlonglong Cliente_repository::contarQuantosRegistrosPorCPFCNPJ(const QString &cpfcnpj){
    if(!DatabaseConnection_service::open()){
        qDebug() << "Erro ao abrir banco de dados contarQuantosRegistros";
        return -1;
    }

    QSqlQuery q(db);

    q.prepare("SELECT COUNT(*) FROM clientes WHERE cpf = :cpf");
    q.bindValue(":cpf", cpfcnpj);

    if (!q.exec()) {
        qDebug() << "Erro ao verificar cliente existente:" << q.lastError();
        return -1;
    }

    q.next();
    qlonglong total = q.value(0).toLongLong();
    db.close();
    return total;
}

bool Cliente_repository::inserir(ClienteDTO cliente){
    bool ehPf = false;
    int indiedest = 1;
    if(cliente.cpf.length() == 14){
        ehPf =false;
    }else{
        ehPf = true;
    }
    if(!cliente.ie.isEmpty()){
        indiedest = 1;
    }else{
        indiedest = 0;
    }

    QString dataFormatada = DataUtil::getDataAgoraUS();
    QSqlQuery query(db);
    if(!DatabaseConnection_service::open()){
        return false;
    }
    query.prepare("INSERT INTO clientes (nome, email, telefone, endereco, cpf, "
                  "data_nascimento, data_cadastro, eh_pf, numero_end, bairro, "
                  "xMun, cMun, uf, cep, indIEDest, ie) VALUES (:nome, :email, :telefone, :endereco, :cpf, "
                  ":data_nascimento, :data_cadastro, :eh_pf, :numero_end, :bairro, "
                  ":xMun, :cMun, :uf, :cep, :indIEDest, :ie)");
    query.bindValue(":nome", cliente.nome);
    query.bindValue(":email", cliente.email);
    query.bindValue(":telefone", cliente.telefone);
    query.bindValue(":endereco", cliente.endereco);
    query.bindValue(":cpf", cliente.cpf);
    query.bindValue(":data_nascimento", cliente.dataNasc);
    query.bindValue(":data_cadastro", dataFormatada);
    query.bindValue(":eh_pf", cliente.ehPf);
    query.bindValue(":numero_end", cliente.endereco);
    query.bindValue(":bairro", cliente.bairro);
    query.bindValue(":xMun", cliente.xMun);
    query.bindValue(":cMun", cliente.cMun);
    query.bindValue(":uf", cliente.uf);
    query.bindValue(":cep", cliente.cep);
    query.bindValue(":indIEDest", cliente.indIeDest);
    query.bindValue(":ie", cliente.ie);

    if(!query.exec()){
        qDebug() << "Query insert Cliente nao funcionou!";
        return false;
    }else{
        qDebug() << "cliente adicionado com sucesso!";

    }
    return true;
}

qlonglong Cliente_repository::getIdFromCPFCNPJ(const QString &cpfcnpj){
    if(!DatabaseConnection_service::open()){
        return -1;
    }

    QSqlQuery q(db);
    qlonglong idcliente;
    q.prepare("SELECT id FROM clientes WHERE cpf = :cnpjemit");
    q.bindValue(":cnpjemit", cpfcnpj);
    if(!q.exec()){
        qDebug() << "nao executou query para achar idcliente em atualizarNotaBanco()";
        return -1;

    }else{
        if (q.next()) {
            idcliente = q.value(0).toLongLong();
            return idcliente;
        }
    }
}

QSqlQueryModel* Cliente_repository::listarClientes(){
    if (!DatabaseConnection_service::open()) {
            qDebug() << "Erro ao abrir banco (listarClientes)";
            return nullptr;
    }

    auto *model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM clientes", db);

    if (model->lastError().isValid()) {
        qDebug() << "Erro SQL:" << model->lastError().text();
    }

    return model;
}

bool Cliente_repository::deletarCliente(qlonglong id){
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco (listarClientes)";
        return false;
    }

    QSqlQuery q(db);

    q.prepare("DELETE FROM clientes WHERE id = :valor1");
    q.bindValue(":valor1", id);
    if(!q.exec()){
        qDebug() << "Não conseguiu deletar cliente";
        return false;
    }else{
        return true;
    }

}

QSqlQueryModel* Cliente_repository::pesquisar(const QString &nome)
{
    if (!DatabaseConnection_service::open()) {
        return nullptr;
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT * FROM clientes "
        "WHERE nome LIKE :nome "
        "ORDER BY id DESC"
        );

    query.bindValue(":nome", "%" + nome + "%");

    if (!query.exec()) {
        qDebug() << "Erro ao executar consulta:" << query.lastError().text();
        return nullptr;
    }

    auto *model = new QSqlQueryModel();
    model->setQuery(std::move(query));

    if (model->lastError().isValid()) {
        qDebug() << "Erro no model:" << model->lastError().text();
        return nullptr;
    }

    return model;
}
