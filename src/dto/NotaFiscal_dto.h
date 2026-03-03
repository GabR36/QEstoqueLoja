#ifndef NOTAFISCAL_DTO_H
#define NOTAFISCAL_DTO_H
#include <QString>

struct NotaFiscalDTO {
    QString cstat;
    qlonglong nnf;
    int serie;
    QString modelo;
    int tpAmb;
    QString xmlPath;
    double valorTotal;
    QString atualizadoEm;
    qlonglong idVenda;
    QString cnpjEmit;
    QString chNfe;
    QString nProt;
    QString cuf;
    QString finalidade;
    bool saida;
    qlonglong idNfRef;
    QString dhEmi;
    qlonglong idEmissorCliente;
};

#endif // NOTAFISCAL_DTO_H
