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
    if (nomeMunicipio.isEmpty()) {
        qWarning() << "Nome do município vazio!";
        return QString();
    }

    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    // Monta a URL com encode correto
    QString urlStr = QString("https://servicodados.ibge.gov.br/api/v1/localidades/municipios?nome=%1")
                         .arg(QUrl::toPercentEncoding(nomeMunicipio));
    QUrl url(urlStr);
    QNetworkRequest request(url);

    QNetworkReply *reply = manager.get(request);
    loop.exec();  // espera a resposta

    // Verifica erro na requisição
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Erro na requisição:" << reply->errorString();
        reply->deleteLater();
        return QString();
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qWarning() << "Resposta não é um array JSON";
        return QString();
    }

    QJsonArray arr = doc.array();
    if (arr.isEmpty()) {
        qWarning() << "Nenhum município encontrado para:" << nomeMunicipio;
        return QString();
    }

    // Percorre todos os resultados e compara o nome exato (case-insensitive)
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        QString nomeRetorno = obj.value("nome").toString();
        if (nomeRetorno.compare(nomeMunicipio, Qt::CaseInsensitive) == 0) {
            int codigo = obj.value("id").toInt();
            return QString::number(codigo);
        }
    }

    qWarning() << "Município exato não encontrado, retornando primeiro resultado";
    // Retorna o primeiro item como fallback
    return QString::number(arr.first().toObject().value("id").toInt());
}
