#ifndef DATAUTIL_H
#define DATAUTIL_H

#include <QObject>
#include <QDateTime>

class DataUtil : public QObject
{
    Q_OBJECT
public:
    explicit DataUtil(QObject *parent = nullptr);

    static QString getDataAgoraUS();
signals:
};

#endif // DATAUTIL_H
