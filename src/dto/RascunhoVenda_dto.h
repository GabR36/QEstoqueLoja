#ifndef RASCUNHOVENDA_DTO_H
#define RASCUNHOVENDA_DTO_H
#include <QString>

struct RascunhoVendaDTO {
    qlonglong idCliente        = -1;
    QString   cpfManual;
    QString   dataHora;
    QString   produtosJson     = "[]";
    // pagamento
    QString   formaPagamento;
    QString   desconto         = "0";
    QString   taxa             = "0";
    QString   recebido         = "0";
    bool      descontoPorcentagem = false;
    // nota fiscal
    int       modeloNf         = 0;
    bool      emitirTodos      = false;
    QString   atualizadoEm;
};

#endif // RASCUNHOVENDA_DTO_H
