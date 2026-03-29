#include "imagestorage_service.h"
#include "../infra/apppath_service.h"
#include <QFile>
#include <QFileInfo>

ImageStorage_service::ImageStorage_service(QObject *parent)
    : QObject{parent}
{}


QString ImageStorage_service::imagesPath()
{
    QString path = AppPath_service::imagesPath();
    return path;
}

QString ImageStorage_service::generateUniqueName(const QString &basePath, const QFileInfo &info)
{
    QString novoCaminho = basePath + "/" + info.fileName();
    int contador = 1;

    while (QFile::exists(novoCaminho)) {
        novoCaminho = basePath + "/" +
                      info.baseName() + "_" +
                      QString::number(contador++) + "." +
                      info.completeSuffix();
    }

    return novoCaminho;
}

QString ImageStorage_service::saveLogoEmpresa(const QString &caminhoOriginal, QString &erro)
{
    if(caminhoOriginal.isEmpty()){
        erro = "Caminho inválido.";
        return "";
    }

    QFileInfo info(caminhoOriginal);
    QString basePath = imagesPath();
    QString novoCaminho = generateUniqueName(basePath, info);

    if(!QFile::copy(caminhoOriginal, novoCaminho)){
        erro = "Não foi possível copiar a imagem.";
        return "";
    }

    // caminho relativo
    return "imagens/" + QFileInfo(novoCaminho).fileName();
}
