#ifndef VENDAS_DTO_H
#define VENDAS_DTO_H
#include <QString>

struct VendasDTO {
    qlonglong id;
    QString clienteNome;
    QString dataHora;
    double total = 0;
    QString formaPagamento;
    double valorRecebido = 0;
    double troco = 0;
    double taxa = 0;
    double valorFinal = 0;
    double desconto = 0;
    bool estaPago;
    qlonglong idCliente;
    QString adicionadoEm;
    QString atualizadoEm;
};

#endif // VENDAS_DTO_H
