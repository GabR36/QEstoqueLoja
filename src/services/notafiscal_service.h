#ifndef NOTAFISCAL_SERVICE_H
#define NOTAFISCAL_SERVICE_H

#include <QObject>
#include "../repository/notafiscal_repository.h"

enum class NotaErro{
    Nenhum,
    Banco,
    Salvar,
    Update
};

class NotaFiscal_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        NotaErro erro = NotaErro::Nenhum;
        QString msg;
    };
    explicit NotaFiscal_service(QObject *parent = nullptr);
    NotaFiscal_service::Resultado salvarResNfe(NotaFiscalDTO resumoNota);
    qlonglong getIdFromChave(QString chnfe);
    NotaFiscal_service::Resultado updateWhereChave(NotaFiscalDTO dto, QString chave);
    qlonglong getIdFromIdVenda(qlonglong idvenda);
    qlonglong getProximoNNF(QString serie, bool tpAmb, qlonglong nnfConfigurado);
    NotaFiscalDTO getNotaNormalFromIdVenda(qlonglong idvenda);
    NotaFiscal_service::Resultado inserir(NotaFiscalDTO nota);
    bool temNotaNormal(qlonglong idvenda);
private:
    notafiscal_repository notaRepo;

signals:
};

#endif // NOTAFISCAL_SERVICE_H
