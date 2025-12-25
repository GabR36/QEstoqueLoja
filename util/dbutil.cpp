#include "dbutil.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariantMap>
#include <QVector>

DBUtil::DBUtil(QObject *parent)
    : QObject{parent}
{

}

QVector<QVariantMap> DBUtil::extrairResultados(QSqlQuery &query)
{
    QVector<QVariantMap> resultados;

    if (!query.isActive()) {
        return resultados;
    }

    QSqlRecord rec = query.record();

    while (query.next()) {
        QVariantMap linha;

        for (int i = 0; i < rec.count(); i++) {
            QString nomeColuna = rec.fieldName(i);
            linha[nomeColuna] = query.value(i);
        }

        resultados.append(linha);
    }

    return resultados;
}
