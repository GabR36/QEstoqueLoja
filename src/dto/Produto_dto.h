#ifndef PRODUTO_DTO_H
#define PRODUTO_DTO_H

#include <QString>

struct ProdutoDTO {
    QString id;
    double quantidade;
    QString descricao;
    double preco;
    QString codigoBarras;
    bool nf;
    QString uCom;
    double precoFornecedor;
    double percentLucro;
    QString ncm;
    QString cest;
    double aliquotaIcms;
    QString csosn;
    QString pis;
};

#endif
