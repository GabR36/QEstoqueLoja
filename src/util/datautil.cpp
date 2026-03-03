#include "datautil.h"

DataUtil::DataUtil(QObject *parent)
    : QObject{parent}
{}

QString DataUtil::getDataAgoraUS(){
    QDateTime dataIngles = QDateTime::currentDateTime();
    QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");

    return dataFormatada;
}
