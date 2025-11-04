#include "buscarcodigomunicipio.h"
#include "buscarcodigomunicipio.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

BuscarCodigoMunicipio::BuscarCodigoMunicipio() {}

QString BuscarCodigoMunicipio::buscarCodigoMunicipio(const QString &nomeMunicipio){
    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    QString urlStr = QString("https://servicodados.ibge.gov.br/api/v1/localidades/municipios?nome=%1")
                         .arg(QUrl::toPercentEncoding(nomeMunicipio));

    // CORREÇÃO: Criar o QUrl primeiro, depois o QNetworkRequest
    QUrl url(urlStr);
    QNetworkRequest request(url);  // Agora está correto!

    QNetworkReply *reply = manager.get(request);
    loop.exec();

    // Restante do código permanece igual...
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Erro na requisição:" << reply->errorString();
        reply->deleteLater();
        return QString();
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray())
        return QString();

    QJsonArray arr = doc.array();
    if (arr.isEmpty())
        return QString();

    QJsonObject obj = arr.first().toObject();
    int codigo = obj["id"].toInt();

    return QString::number(codigo);
}
