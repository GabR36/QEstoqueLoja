#ifndef EVENTOFISCAL_DTO_H
#define EVENTOFISCAL_DTO_H

#include <QString>

struct EventoFiscalDTO {

    QString tipoEvento;
    int idLote;
    QString cstat;
    QString justificativa;
    QString xmlPath;
    QString nProt;
    qlonglong idNf;
    QString atualizadoEm;
    QString codigo;
};

#endif
