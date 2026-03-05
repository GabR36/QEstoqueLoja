#ifndef PRODUTOPARANFE_DTO_H
#define PRODUTOPARANFE_DTO_H

#include <qstring.h>

struct ProdutoParaNFeDTO {
    qlonglong idProduto;
    double quantidade;
    double valorUnitario;
    double valorTotal;
    QString codigoBarras;
    QString uCom;
    QString ncm;
    QString cest;
    QString csosn;
    QString pis;
    double aliquotaIcms;
    QString cfop;
    double desconto;
    QString descricao;
}
#endif // PRODUTOPARANFE_DTO_H
