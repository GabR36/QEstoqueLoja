#ifndef ZIPUTIL_H
#define ZIPUTIL_H

#include <quazip/quazip.h>

class ZipUtil
{
public:
    ZipUtil();
    static bool adicionarArquivoZip(QuaZip &zip, QString caminhoArquivo);
};

#endif // ZIPUTIL_H
