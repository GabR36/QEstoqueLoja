#ifndef NOTAFISCAL_DTO_H
#define NOTAFISCAL_DTO_H
#include <QString>

struct NotaFiscalDTO {
    QString cstat;
    qlonglong nnf = 0;
    int serie = 0;
    QString modelo;
    int tpAmb;
    QString xmlPath;
    double valorTotal = 0;
    QString atualizadoEm;
    qlonglong idVenda = 0;
    QString cnpjEmit;
    QString chNfe;
    QString nProt;
    QString cuf;
    QString finalidade;
    bool saida;
    qlonglong idNfRef = 0;
    QString dhEmi;
    qlonglong idEmissorCliente = 0;
    QString adicionadoEm;
};

#endif // NOTAFISCAL_DTO_H
