#ifndef CONFIGDB_DTO_H
#define CONFIGDB_DTO_H


#include <QString>

struct ConfigDbDTO{
    //DB
    int driverDB = 0;
    QString ipHostDB;
    QString portaDB;
    QString nomeDB;
    QString userDB;
    QString senhaDB;
    QString pathPastaSqliteDB;
    QString pathPastaPostgreDB;
};
#endif // CONFIGDB_DTO_H
