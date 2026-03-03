#ifndef IMAGESTORAGE_SERVICE_H
#define IMAGESTORAGE_SERVICE_H

#include <QObject>
#include <QFile>
#include <qfileinfo.h>

class ImageStorage_service : public QObject
{
    Q_OBJECT
public:
    explicit ImageStorage_service(QObject *parent = nullptr);

    QString saveLogoEmpresa(const QString &caminhoOriginal, QString &erro);
signals:
private:
    QString imagesPath();
    QString generateUniqueName(const QString &basePath, const QFileInfo &info);
};

#endif // IMAGESTORAGE_SERVICE_H
