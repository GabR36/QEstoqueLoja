#ifndef DBUTIL_H
#define DBUTIL_H

#include <QObject>
#include <QSqlQuery>

class DBUtil : public QObject
{
    Q_OBJECT
public:
    explicit DBUtil(QObject *parent = nullptr);

    static QVector<QVariantMap> extrairResultados(QSqlQuery &query);
signals:
};

#endif // DBUTIL_H
