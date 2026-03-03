#ifndef ENTRADAVENDA_DTO_H
#define ENTRADAVENDA_DTO_H
#include <QString>

struct EntradaVendaDTO {
    qlonglong idVenda;
    double total;
    QString dataHora;
    QString formaPagamento;
    double valorRecebido;
    double troco;
    double taxa;
    double valorFinal;
    double desconto;
};

#endif // ENTRADAVENDA_DTO_H
