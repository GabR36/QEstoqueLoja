#ifndef PRODUTONOTA_DTO_H
#define PRODUTONOTA_DTO_H

#include <qstring.h>

struct ProdutoNotaDTO {
    qlonglong id;
    double quantidade;
    QString descricao;
    double preco = 0;
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
    double aliquotaIcms = 0;
    QString csosn;
    QString pis;
};

#endif // PRODUTONOTA_DTO_H
