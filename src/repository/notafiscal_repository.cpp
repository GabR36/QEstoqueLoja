#include "notafiscal_repository.h"
#include "../infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>
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
                  "cnpjemit, chnfe, nprot, cuf, finalidade, saida, nnf, serie, dhemi, adicionado_em) VALUES (:cstat, :modelo,"
                  " :tpamb, :xmlpath, :vnf, :atualizadoem, :cnpjemit, :chnf, :nprot, :cuf, :finalidade, "
                  ":saida, :nnf, :serie, :dhemi, :adicionadoem)");
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
    query.bindValue(":adicionadoem", dataFormatada);

    if(!query.exec()){
        qDebug() << "ERRO INSERT notas_fiscais:" << query.lastError().text();
        db.close();
        return false;
    } else {
        qDebug() << "Resumo nota salvo com sucesso!";
        db.close();
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
        db.close();
        return -1;
    }
    db.close();
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
        db.close();
        return false;
    }else{
        qDebug() << "Nota fiscal atualizada com sucesso!";
    }
    qDebug() << "Linhas afetadas:" << q.numRowsAffected();
    db.close();
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


qlonglong notafiscal_repository::getProximoNNF55(QString serie, bool tpAmb, qlonglong nnfConfigurado){
    if(!DatabaseConnection_service::open()){
        qDebug() << "Banco nao abriu getProximoNNF()";
        return -1;
    }

    QSqlQuery query(db);

    query.prepare(
        "SELECT nnf FROM notas_fiscais "
        "WHERE modelo = :modelo "
        "AND serie = :serie "
        "AND tp_amb = :tp_amb "
        "AND finalidade != 'ENTRADA EXTERNA' "
        "ORDER BY nnf DESC "
        "LIMIT 1"
        );

    query.bindValue(":modelo", "55");
    query.bindValue(":serie", serie);
    query.bindValue(":tp_amb", tpAmb);

    if(!query.exec()){
        qWarning() << "Erro na consulta NNF:" << query.lastError().text();
        db.close();
        return -1;
    }

    if(query.next()){
        int ultimoNNF = query.value(0).toInt();
        db.close();
        return ultimoNNF + 1;
    }

    // Se não encontrou nenhuma nota
    if(nnfConfigurado > 0){
        db.close();
        return nnfConfigurado + 1;
    }
    db.close();
    return 1; // fallback final
}

qlonglong notafiscal_repository::getProximoNNF65(QString serie, bool tpAmb, qlonglong nnfConfigurado){
    if(!DatabaseConnection_service::open()){
        qDebug() << "Banco nao abriu getProximoNNF()";
        return -1;
    }

    QSqlQuery query(db);

    query.prepare(
        "SELECT nnf FROM notas_fiscais "
        "WHERE modelo = :modelo "
        "AND serie = :serie "
        "AND tp_amb = :tp_amb "
        "AND finalidade != 'ENTRADA EXTERNA' "
        "ORDER BY nnf DESC "
        "LIMIT 1"
        );

    query.bindValue(":modelo", "65");
    query.bindValue(":serie", serie);
    query.bindValue(":tp_amb", tpAmb);

    if(!query.exec()){
        qWarning() << "Erro na consulta NNF:" << query.lastError().text();
        db.close();
        return -1;
    }

    if(query.next()){
        int ultimoNNF = query.value(0).toInt();
        db.close();
        return ultimoNNF + 1;
    }

    // Se não encontrou nenhuma nota
    if(nnfConfigurado > 0){
        db.close();
        return nnfConfigurado + 1;
    }
    db.close();
    return 1; // fallback final
}

NotaFiscalDTO notafiscal_repository::getNotaNormalFromIdVenda(qlonglong idvenda){
    NotaFiscalDTO nota;
    if(!DatabaseConnection_service::open()){
        qDebug() << "Banco nao abriu getNotaNormalFromIdVenda()";
        return nota;
    }

    QSqlQuery query(db);
    query.prepare("SELECT cstat, nnf, serie, modelo, tp_amb, xml_path, valor_total, atualizado_em, "
                  "id_venda, cnpjemit, chnfe, nprot, cuf, finalidade, saida, id_nf_ref, dhemi, "
                  "id_emissorcliente, adicionado_em FROM notas_fiscais WHERE "
                  "id_venda = :idvenda AND finalidade = 'NORMAL'");
    query.bindValue(":idvenda", idvenda);

    if(!query.exec()){
        qDebug() << "Query nao rodou getNotaNormalFromIdVenda";
        db.close();
        return nota;
    }

    while(query.next()){
        nota.atualizadoEm = query.value("atualizado_em").toString();
        nota.chNfe = query.value("chnfe").toString();
        nota.cnpjEmit = query.value("cnpjemit").toString();
        nota.cstat = query.value("cstat").toString();
        nota.cuf = query.value("cuf").toString();
        nota.dhEmi = query.value("dhemi").toString();
        nota.finalidade = query.value("finalidade").toString();
        nota.idNfRef = query.value("id_nf_ref").toLongLong();
        nota.idEmissorCliente = query.value("id_emissorcliente").toLongLong();
        nota.idVenda = query.value("id_venda").toLongLong();
        nota.modelo = query.value("modelo").toString();
        nota.nnf = query.value("nnf").toLongLong();
        nota.nProt = query.value("nprot").toString();
        nota.saida = query.value("saida").toBool();
        nota.serie = query.value("serie").toInt();
        nota.tpAmb = query.value("tpamb").toInt();
        nota.valorTotal = query.value("valor_total").toDouble();
        nota.xmlPath = query.value("xml_path").toString();
        nota.adicionadoEm = query.value("adicionado_em").toString();
    }
    db.close();
    return nota;
}

bool notafiscal_repository::inserir(NotaFiscalDTO nota){
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return false;
    }
    QString dataFormatada = DataUtil::getDataAgoraUS();
    QSqlQuery query(db);

    // QString dhemi = nota.dhEmi;
    // QString dhemiFormatada = dt.toString("yyyy-MM-dd HH:mm:ss");
    // qDebug() << "dhemi formatada:" << dhemiFormatada;

    query.prepare("INSERT INTO notas_fiscais (cstat, nnf, serie, modelo, tp_amb, xml_path, valor_total, "
                  "atualizado_em, id_venda, "
                  "cnpjemit, chnfe, nprot, cuf, finalidade, saida, id_nf_ref, dhemi, "
                  "id_emissorcliente, adicionado_em) "
                  "VALUES (:cstat, :nnf, :serie, :modelo, :tpamb, :xmlpath, :vnf, :atualizadoem, :idvenda, "
                  ":cnpjemit, :chnf, :nprot, :cuf, :finalidade, :saida, :idnfref, :dhemi, :idemissor, "
                  ":adicionadoem)");
    query.bindValue(":cstat", nota.cstat);
    query.bindValue(":nnf", nota.nnf);
    query.bindValue(":serie", nota.serie);
    query.bindValue(":modelo", nota.modelo);
    query.bindValue(":tpamb", nota.tpAmb);
    query.bindValue(":xmlpath", nota.xmlPath);
    query.bindValue(":vnf", nota.valorTotal);
    query.bindValue(":atualizadoem", dataFormatada);
    query.bindValue(":idvenda", nota.idVenda);
    query.bindValue(":cnpjemit", nota.cnpjEmit);
    query.bindValue(":chnf", nota.chNfe);
    query.bindValue(":nprot", nota.nProt);
    query.bindValue(":cuf", nota.cuf);
    query.bindValue(":finalidade", nota.finalidade);
    query.bindValue(":saida", nota.saida);
    query.bindValue(":idnfref", nota.idNfRef);
    query.bindValue(":dhemi", nota.dhEmi);
    query.bindValue(":idemissor", nota.idEmissorCliente);
    query.bindValue(":adicionadoem", dataFormatada);


    if(!query.exec()){
        qDebug() << "ERRO INSERT notas_fiscais:" << query.lastError().text();
        db.close();
        return false;
    } else {
        qDebug() << "Nota salvo com sucesso!";
        db.close();
        return true;
    }
}

void notafiscal_repository::listarEntradas(QSqlQueryModel *model, const QString &de, const QString &ate)
{
    if (!model) return;

    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco listarEntradas()";
        return;
    }

    QString sql = R"(
        SELECT
            c.nome AS emitente,
            n.valor_total AS valor,
            n.dhemi AS emissao,
            n.cnpjemit AS cnpj,
            n.modelo AS modelo,
            n.chnfe AS chave,
            n.cstat AS cstat,
            n.id AS id_nf
        FROM notas_fiscais n
        LEFT JOIN clientes c
            ON c.id = n.id_emissorcliente
        WHERE n.finalidade = 'ENTRADA EXTERNA'
          AND n.cstat IN (100, 150)
    )";

    if (!de.isEmpty() && !ate.isEmpty())
        sql += " AND n.dhemi BETWEEN '" + de + "' AND '" + ate + "'";

    sql += " ORDER BY n.dhemi DESC";

    model->setQuery(sql, db);

    if (model->lastError().isValid())
        qDebug() << "Erro SQL listarEntradas:" << model->lastError().text();

    db.close();
}

qlonglong notafiscal_repository::getIdNotaNormalFromIdVenda(qlonglong idvenda){
    if(!DatabaseConnection_service::open()){
        qDebug() << "db nao aberto ao salvar resumo nota";
        return -1;
    }
    QSqlQuery query(db);
    query.prepare("SELECT id FROM notas_fiscais WHERE id_venda = :idvenda AND finalidade = 'NORMAL'");
    query.bindValue(":idvenda", idvenda);
    if(!query.exec()){
        qDebug() << "Query nao rodou getIdNotaNormalFromIdVenda";
        db.close();
        return -1;
    }
    qlonglong id = -1;
    if(query.next()){
        id = query.value(0).toLongLong();
    }
    db.close();
    return id;

}

NotaFiscalDTO notafiscal_repository::getNotaById(qlonglong id){
    NotaFiscalDTO nota;
    if(!DatabaseConnection_service::open()){
        qDebug() << "Banco nao abriu getNotaById()";
        return nota;
    }

    QSqlQuery query(db);
    query.prepare("SELECT cstat, nnf, serie, modelo, tp_amb, xml_path, valor_total, atualizado_em, "
                  "id_venda, cnpjemit, chnfe, nprot, cuf, finalidade, saida, id_nf_ref, dhemi, "
                  "id_emissorcliente, adicionado_em FROM notas_fiscais WHERE id = :id");
    query.bindValue(":id", id);

    if(!query.exec()){
        qDebug() << "Query nao rodou getNotaById";
        db.close();
        return nota;
    }

    if(query.next()){
        nota.atualizadoEm      = query.value("atualizado_em").toString();
        nota.chNfe             = query.value("chnfe").toString();
        nota.cnpjEmit          = query.value("cnpjemit").toString();
        nota.cstat             = query.value("cstat").toString();
        nota.cuf               = query.value("cuf").toString();
        nota.dhEmi             = query.value("dhemi").toString();
        nota.finalidade        = query.value("finalidade").toString();
        nota.idNfRef           = query.value("id_nf_ref").toLongLong();
        nota.idEmissorCliente  = query.value("id_emissorcliente").toLongLong();
        nota.idVenda           = query.value("id_venda").toLongLong();
        nota.modelo            = query.value("modelo").toString();
        nota.nnf               = query.value("nnf").toLongLong();
        nota.nProt             = query.value("nprot").toString();
        nota.saida             = query.value("saida").toBool();
        nota.serie             = query.value("serie").toInt();
        nota.tpAmb             = query.value("tp_amb").toInt();
        nota.valorTotal        = query.value("valor_total").toDouble();
        nota.xmlPath           = query.value("xml_path").toString();
        nota.adicionadoEm           = query.value("adicionado_em").toString();

    }

    db.close();
    return nota;
}

QMap<QString, int> notafiscal_repository::contarPorFinalidade(QDateTime dtIni, QDateTime dtFim, int tpAmb)
{
    QMap<QString, int> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. contarPorFinalidade";
        return resultado;
    }

    QSqlQuery q(db);
    q.prepare(R"(
        SELECT finalidade, COUNT(*)
        FROM notas_fiscais
        WHERE dhemi BETWEEN :ini AND :fim
        AND tp_amb = :tpamb
        GROUP BY finalidade
    )");
    q.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":tpamb", tpAmb);

    if (q.exec()) {
        while (q.next())
            resultado[q.value(0).toString()] = q.value(1).toInt();
    } else {
        qDebug() << "Erro contarPorFinalidade:" << q.lastError().text();
    }

    db.close();
    return resultado;
}

QList<QPair<QString, QString>> notafiscal_repository::buscarXmlsPorPeriodo(QDateTime dtIni, QDateTime dtFim, int tpAmb)
{
    QList<QPair<QString, QString>> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarXmlsPorPeriodo (notas)";
        return resultado;
    }

    QSqlQuery q(db);
    q.prepare(R"(
        SELECT finalidade, xml_path
        FROM notas_fiscais
        WHERE dhemi BETWEEN :ini AND :fim
        AND tp_amb = :tpamb
    )");
    q.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":tpamb", tpAmb);

    if (q.exec()) {
        while (q.next())
            resultado.append({q.value("finalidade").toString(), q.value("xml_path").toString()});
    } else {
        qDebug() << "Erro buscarXmlsPorPeriodo (notas):" << q.lastError().text();
    }

    db.close();
    return resultado;
}

QList<NotaFiscalDTO> notafiscal_repository::buscarPorPeriodo(QDateTime dtIni, QDateTime dtFim, int tpAmb)
{
    QList<NotaFiscalDTO> resultado;
    if (!DatabaseConnection_service::open()) {
        qDebug() << "Erro ao abrir banco. buscarPorPeriodo";
        return resultado;
    }

    QSqlQuery q(db);
    q.prepare(R"(
        SELECT nnf, dhemi, chnfe, valor_total, finalidade, cstat
        FROM notas_fiscais
        WHERE dhemi BETWEEN :ini AND :fim
        AND tp_amb = :tpamb
        AND finalidade != 'ENTRADA EXTERNA'
        ORDER BY dhemi
    )");
    q.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":tpamb", tpAmb);

    if (q.exec()) {
        while (q.next()) {
            NotaFiscalDTO nota;
            nota.nnf        = q.value("nnf").toLongLong();
            nota.dhEmi      = q.value("dhemi").toString();
            nota.chNfe      = q.value("chnfe").toString();
            nota.valorTotal = q.value("valor_total").toDouble();
            nota.finalidade = q.value("finalidade").toString();
            nota.cstat      = q.value("cstat").toString();
            resultado.append(nota);
        }
    } else {
        qDebug() << "Erro buscarPorPeriodo:" << q.lastError().text();
    }

    db.close();
    return resultado;
}

void notafiscal_repository::listarMonitor(QSqlQueryModel *model, const QStringList &finalidades)
{
    if(!model) return;

    if(!DatabaseConnection_service::open()){
        qDebug() << "Erro ao abrir banco listarMonitor()";
        return;
    }

    QStringList placeholders;
    for(const QString &f : finalidades)
        placeholders << QString("'%1'").arg(f);

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
        FROM notas_fiscais
        WHERE finalidade IN ()" + placeholders.join(", ") + R"()
        ORDER BY dhemi DESC
    )";

    model->setQuery(sql, db);

    if(model->lastError().isValid())
        qDebug() << "Erro SQL listarMonitor:" << model->lastError().text();

    db.close();
}
