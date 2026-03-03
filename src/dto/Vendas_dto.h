#ifndef VENDAS_DTO_H
#define VENDAS_DTO_H
#include <QString>

struct VendasDTO {
    QString clienteNome;
    QString dataHora;
    double total;
    QString formaPagamento;
    double valorRecebido;
    double troco;
    double taxa;
    double valorFinal;
    double desconto;
    bool estaPago;
    qlonglong idCliente;
};

#endif // VENDAS_DTO_H
