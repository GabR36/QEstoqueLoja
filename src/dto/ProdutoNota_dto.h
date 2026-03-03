#ifndef PRODUTONOTA_DTO_H
#define PRODUTONOTA_DTO_H

#include <qstring.h>

struct ProdutoNotaDTO {
    double quantidade;
    QString descricao;
    double preco;
    QString codigoBarras;
    QString uCom;
    QString cfop;
    int nitem;
    qlonglong idNf;
    QString status;
    QString cstIcms;
    bool temSt;
    qlonglong idNfDevol;
    bool adicionado;
    QString ncm;
    double aliquotaIcms;
    QString csosn;
    QString pis;
};

#endif // PRODUTONOTA_DTO_H
