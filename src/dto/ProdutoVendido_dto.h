#ifndef PRODUTOVENDIDO_DTO_H
#define PRODUTOVENDIDO_DTO_H

#include <QString>

struct ProdutoVendidoDTO {
    qlonglong idProduto;
    qlonglong idVenda;
    double quantidade;
    double precoVendido;
    // campos nao pertencentes a tabela do banco de dados
    QString descricao;
};

#endif // PRODUTOVENDIDO_DTO_H
