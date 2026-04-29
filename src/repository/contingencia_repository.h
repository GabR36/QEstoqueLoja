#ifndef CONTINGENCIA_REPOSITORY_H
#define CONTINGENCIA_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "../dto/NotaFiscal_dto.h"

class ContingenciaRepository : public QObject
{
    Q_OBJECT
public:
    /**
     * @param db  Conexão já aberta. Deve pertencer ao thread que usa este repositório.
     *            Em testes, passe um banco SQLite in-memory.
     */
    explicit ContingenciaRepository(const QSqlDatabase &db, QObject *parent = nullptr);

    QList<NotaFiscalDTO> buscarPendentes();
    bool atualizar(const QString &chNfe, const QString &cstat, const QString &nProt);

private:
    QSqlDatabase m_db;
};

#endif // CONTINGENCIA_REPOSITORY_H
