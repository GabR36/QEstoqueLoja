#include "contingencia_repository.h"
#include "../util/datautil.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ContingenciaRepository::ContingenciaRepository(const QSqlDatabase &db, QObject *parent)
    : QObject{parent}
    , m_db{db}
{}

QList<NotaFiscalDTO> ContingenciaRepository::buscarPendentes()
{
    QList<NotaFiscalDTO> lista;

    QSqlQuery q(m_db);
    q.prepare("SELECT cstat, nnf, serie, modelo, tp_amb, xml_path, valor_total, "
              "id_venda, cnpjemit, chnfe, nprot, cuf, finalidade, saida, id_nf_ref, dhemi "
              "FROM notas_fiscais WHERE cstat = 'CONTINGENCIA'");

    if (q.exec()) {
        while (q.next()) {
            NotaFiscalDTO nota;
            nota.cstat      = q.value("cstat").toString();
            nota.chNfe      = q.value("chnfe").toString();
            nota.xmlPath    = q.value("xml_path").toString();
            nota.modelo     = q.value("modelo").toString();
            nota.nnf        = q.value("nnf").toLongLong();
            nota.nProt      = q.value("nprot").toString();
            nota.finalidade = q.value("finalidade").toString();
            nota.dhEmi      = q.value("dhemi").toString();
            lista.append(nota);
        }
    } else {
        qDebug() << "ContingenciaRepository::buscarPendentes:" << q.lastError().text();
    }

    return lista;
}

bool ContingenciaRepository::atualizar(const QString &chNfe,
                                       const QString &cstat,
                                       const QString &nProt)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE notas_fiscais SET cstat = :cstat, nprot = :nprot, atualizado_em = :atualizadoem "
              "WHERE chnfe = :chnfe");
    q.bindValue(":cstat",        cstat);
    q.bindValue(":nprot",        nProt);
    q.bindValue(":chnfe",        chNfe);
    q.bindValue(":atualizadoem", DataUtil::getDataAgoraUS());

    bool ok = q.exec();
    if (!ok)
        qDebug() << "ContingenciaRepository::atualizar:" << q.lastError().text();
    else
        qDebug() << "ContingenciaRepository: contingência atualizada —" << chNfe << "cStat:" << cstat;
    return ok;
}
