#ifndef NOTAFISCAL_REPOSITORY_H
#define NOTAFISCAL_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQueryModel>
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
    qlonglong getProximoNNF55(QString serie, bool tpAmb, qlonglong nnfConfigurado);
    NotaFiscalDTO getNotaNormalFromIdVenda(qlonglong idvenda);
    NotaFiscalDTO getNotaById(qlonglong id);
    bool inserir(NotaFiscalDTO nota);
    qlonglong getIdNotaNormalFromIdVenda(qlonglong idvenda);
    qlonglong getProximoNNF65(QString serie, bool tpAmb, qlonglong nnfConfigurado);
    void listarEntradas(QSqlQueryModel *model, const QString &de = "", const QString &ate = "");
private:
    QSqlDatabase db;

signals:
};

#endif // NOTAFISCAL_REPOSITORY_H
