#ifndef NFRETORNO_DTO_H
#define NFRETORNO_DTO_H

#include <QString>

struct NFRetornoDTO {
    QString cstat;
    qlonglong nnf;
    int serie;
    QString modelo;
    int tpAmb;
    QString xmlPath;
    double valorTotal;
    QString cnpjEmit;
    QString chNfe;
    QString nProt;
    QString cuf;
    QString finalidade;
    QString dhEmi;
    QString xMotivo;
    QString msg;
}
#endif // NFRETORNO_DTO_H
