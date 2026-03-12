#ifndef NOTAFISCAL_SERVICE_H
#define NOTAFISCAL_SERVICE_H

#include <QObject>
#include "../repository/notafiscal_repository.h"
#include "config_service.h"

enum class NotaErro{
    Nenhum,
    Banco,
    Salvar,
    Update
};

enum class ModeloNota{
    NFe,
    NFCe
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
    qlonglong getProximoNNF(bool tpAmb, ModeloNota mod);
    qlonglong getProximoNNF(QString serie, bool tpAmb, ModeloNota mod);
    NotaFiscalDTO getNotaNormalFromIdVenda(qlonglong idvenda);
    NotaFiscal_service::Resultado inserir(NotaFiscalDTO nota);
    bool temNotaNormal(qlonglong idvenda);
private:
    notafiscal_repository notaRepo;
    Config_service confServ;
    ConfigDTO confDTO;

signals:
};

#endif // NOTAFISCAL_SERVICE_H
