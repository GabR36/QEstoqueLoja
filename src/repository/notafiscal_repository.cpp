#include "notafiscal_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include "../util/datautil.h"

notafiscal_repository::notafiscal_repository(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
}


bool notafiscal_repository::salvarResNFe(NotaFiscalDTO resumoNota){
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return false;
    }
    QString dataFormatada = DataUtil::getDataAgoraUS();
    QSqlQuery query(db);

    QString dhemi = resumoNota.dhEmi;
    QDateTime dt = QDateTime::fromString(dhemi, "dd/MM/yyyy HH:mm:ss");
    QString dhemiFormatada = dt.toString("yyyy-MM-dd HH:mm:ss");

    query.prepare("INSERT INTO notas_fiscais (cstat, modelo, tp_amb, xml_path, valor_total, atualizado_em,"
                  "cnpjemit, chnfe, nprot, cuf, finalidade, saida, nnf, serie, dhemi) VALUES (:cstat, :modelo,"
                  " :tpamb, :xmlpath, :vnf, :atualizadoem, :cnpjemit, :chnf, :nprot, :cuf, :finalidade, "
                  ":saida, :nnf, :serie, :dhemi)");
    query.bindValue(":cstat", resumoNota.cstat);
    query.bindValue(":modelo", "55");
    query.bindValue(":tpamb", resumoNota.tpAmb);
    query.bindValue(":xmlpath", resumoNota.xmlPath);
    query.bindValue(":vnf", resumoNota.valorTotal);
    query.bindValue(":atualizadoem", dataFormatada);
    query.bindValue(":cnpjemit", resumoNota.cnpjEmit);
    query.bindValue(":chnf", resumoNota.chNfe);
    query.bindValue(":nprot", resumoNota.nProt);
    query.bindValue(":cuf", "");
    query.bindValue(":finalidade", resumoNota.finalidade);
    query.bindValue(":saida", "0");
    query.bindValue(":nnf", "0");
    query.bindValue(":serie", "");
    query.bindValue(":dhemi", dhemiFormatada);

    if(!query.exec()){
        qDebug() << "ERRO INSERT notas_fiscais:" << query.lastError().text();
        return false;
    } else {
        qDebug() << "Resumo nota salvo com sucesso!";
        return true;
    }
}


qlonglong notafiscal_repository::getIdFromChave(QString chnfe){
    if(!DatabaseConnection_service::open()){
            qDebug() << "Banco nao abriu getidfromchave()";
            return -1;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id FROM notas_fiscais WHERE chnfe = :chnfe");
    query.bindValue(":chnfe", chnfe);
    QString id_nf = "";
    if(query.exec()){
        while(query.next()){
            id_nf = query.value(0).toString();
        }
    }else{
        qDebug() << "nao rodou query select idnf";
        return -1;
    }
    return id_nf.toLongLong();
}

bool notafiscal_repository::updateWhereChave(NotaFiscalDTO dto, QString chave){
    if(!DatabaseConnection_service::open()){
        qDebug() << "Banco nao abriu updateWhereChave()";
        return false;
    }

    QSqlQuery q(db);
    qDebug() << "Data DHEMI updateWHERECHAVE: " << dto.dhEmi;
    QString dhemi = dto.dhEmi;
    QDateTime dt = QDateTime::fromString(dhemi, Qt::ISODate);//formato do dhemi no xml
    QString dhemiFormatada = dt.toString("yyyy-MM-dd HH:mm:ss");


    QString dataAgoraFormatada = DataUtil::getDataAgoraUS();
    q.prepare("UPDATE notas_fiscais SET "
              "cstat = :cstat, "
              "nnf = :nnf, "
              "serie = :serie, "
              "modelo = :modelo, "
              "tp_amb = :tp_amb, "
              "xml_path = :xml_path, "
              "valor_total = :valor_total, "
              "cnpjemit = :cnpjemit, "
              "nprot = :nprot, "
              "cuf = :cuf, "
              "atualizado_em = :atualizadoem, "
              "finalidade = :finalidade, "
              "saida = :saida, "
              "id_emissorcliente = :idcliente, "
              "dhemi = :dhemi "
              "WHERE chnfe = :chnfe");
    q.bindValue(":cstat", dto.cstat);
    q.bindValue(":nnf", dto.nnf);
    q.bindValue(":serie", dto.serie);
    q.bindValue(":modelo", dto.modelo);
    q.bindValue(":tp_amb", dto.tpAmb);
    q.bindValue(":xml_path", dto.xmlPath);
    q.bindValue(":valor_total", dto.valorTotal);
    q.bindValue(":cnpjemit", dto.cnpjEmit);
    q.bindValue(":nprot", dto.nProt);
    q.bindValue(":cuf", dto.cuf);
    q.bindValue(":chnfe", chave);
    q.bindValue(":atualizadoem", dataAgoraFormatada);
    q.bindValue(":finalidade", "ENTRADA EXTERNA");
    q.bindValue(":saida", "0");
    q.bindValue(":idcliente", dto.idEmissorCliente);
    q.bindValue(":dhemi", dhemiFormatada);


    if (!q.exec()){
        qDebug() << "Erro ao atualizar NF:" << q.lastError();
        return false;
    }else{
        qDebug() << "Nota fiscal atualizada com sucesso!";
    }
    qDebug() << "Linhas afetadas:" << q.numRowsAffected();
    return true;
}

qlonglong notafiscal_repository::getIdFromIdVenda(qlonglong idvenda){
    if(!DatabaseConnection_service::open()){
        qDebug() << "Banco nao abriu getIdFromIdVenda()";
        return -1;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id FROM notas_fiscais WHERE id_venda = :idvenda");
    query.bindValue(":idvenda", idvenda);

    if(!query.exec()){
        qDebug() << "Query getIdFromIdVenda não rodou";
        db.close();
        return -1;
    }


    qlonglong idRetorno = -1;

    if(!query.next()){
        return -1;
    }else{
        idRetorno = query.value(0).toLongLong();
    }
    return idRetorno;
}
