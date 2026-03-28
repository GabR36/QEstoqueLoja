#ifndef PRODUTOVENDIDO_DTO_H
#define PRODUTOVENDIDO_DTO_H

#include <QString>

struct ProdutoVendidoDTO {
    qlonglong idProduto;
    qlonglong idVenda;
    double quantidade = 0;
    double precoVendido = 0;
    // campos nao pertencentes a tabela do banco de dados
    QString descricao;
    bool emitidoNf;
    QString adicionadoEm;
    QString atualizadoEm;
};

#endif // PRODUTOVENDIDO_DTO_H
