#ifndef FISCALEMITTER_SERVICE_H
#define FISCALEMITTER_SERVICE_H

#include <QObject>
#include "../nota/nfeacbr.h"
#include "../dto/ProdutoVendido_dto.h"
#include "notafiscal_service.h"
#include "../dto/Vendas_dto.h"

enum class FiscalEmitterErro{
    Nenhum,
    Banco,
    Salvar,
    Update,
    Inserir,
    ErroAoEnviar,
    Recusado,
    InfoInsuficiente
};

class FiscalEmitter_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        FiscalEmitterErro erro = FiscalEmitterErro::Nenhum;
        QString msg;
    };
    explicit FiscalEmitter_service(QObject *parent = nullptr);
    FiscalEmitter_service::Resultado enviarNfeDevolucaoPadrao(qlonglong idvenda, QList<ProdutoVendidoDTO> listaProds);
private:
    NotaFiscal_service notaServ;

signals:
};

#endif // FISCALEMITTER_SERVICE_H
