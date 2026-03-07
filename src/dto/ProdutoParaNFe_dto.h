#ifndef PRODUTOPARANFE_DTO_H
#define PRODUTOPARANFE_DTO_H

#include <qstring.h>

struct ProdutoParaNFeDTO {
    qlonglong idProduto;
    double quantidade = 0;
    double valorUnitario = 0;
    double valorTotal = 0;
    QString codigoBarras;
    QString uCom;
    QString ncm;
    QString cest;
    QString csosn;
    QString pis;
    double aliquotaIcms = 0;
    QString cfop;
    double desconto = 0;
    QString descricao;
};
#endif // PRODUTOPARANFE_DTO_H
