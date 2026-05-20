#ifndef RASCUNHOVENDASAVE_DTO_H
#define RASCUNHOVENDASAVE_DTO_H

#include <QString>
#include <QList>
#include "ProdutoVendido_dto.h"

struct RascunhoVendaSaveDTO {
    qlonglong idCliente           = -1;
    QString   cpfManual;
    QString   dataHora;
    QList<ProdutoVendidoDTO> produtos;
    // pagamento
    QString   formaPagamento;
    QString   desconto            = "0";
    QString   taxa                = "0";
    QString   recebido            = "0";
    bool      descontoPorcentagem = false;
    // nota fiscal
    int       modeloNf            = 0;
    bool      emitirTodos         = false;
};

#endif // RASCUNHOVENDASAVE_DTO_H
