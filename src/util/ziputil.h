#ifndef ZIPUTIL_H
#define ZIPUTIL_H

#include <QString>

class QuaZip;

class ZipUtil
{
public:
    ZipUtil();
    static bool adicionarArquivoZip(QuaZip &zip, QString caminhoArquivo);
    static bool comprimirDiretorio(const QString &dirPath, const QString &zipPath);
};

#endif // ZIPUTIL_H
