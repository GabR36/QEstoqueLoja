#ifndef NOTAFISCAL_REPOSITORY_H
#define NOTAFISCAL_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <../dto/NotaFiscal_dto.h>

class notafiscal_repository : public QObject
{
    Q_OBJECT
public:
    explicit notafiscal_repository(QObject *parent = nullptr);
    bool salvarResNFe(NotaFiscalDTO resumoNota);
    qlonglong getIdFromChave(QString chnfe);
    bool updateWhereChave(NotaFiscalDTO dto, QString chave);
    qlonglong getIdFromIdVenda(qlonglong idvenda);
private:
    QSqlDatabase db;

signals:
};

#endif // NOTAFISCAL_REPOSITORY_H
