#include "ziputil.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <qfile.h>
#include <qfileinfo.h>


ZipUtil::ZipUtil() {}

bool ZipUtil::adicionarArquivoZip(QuaZip &zip, QString caminhoArquivo) {
    QFile file(caminhoArquivo);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Não foi possível abrir:" << caminhoArquivo;
    }
    QFileInfo info(caminhoArquivo);

    QuaZipFile zipFile(&zip);

    QuaZipNewInfo zipInfo(info.fileName(), info.filePath());

    if (!zipFile.open(QIODevice::WriteOnly, zipInfo)) {
        qDebug() << "Erro ao adicionar arquivo ao ZIP:" << zipFile.getZipError();
    }

    zipFile.write(file.readAll());
    zipFile.close();
    return true;
}
