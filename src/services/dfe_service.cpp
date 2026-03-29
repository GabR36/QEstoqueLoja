#include "dfe_service.h"
#include "../util/manifestadordfe.h"
#include <QDateTime>

Dfe_service::Dfe_service(QObject *parent)
    : QObject{parent}
{

}

QString Dfe_service::getUltimaIdentificaçãoUsada(){
    return dfeRepo.getUltimaIdentificaçãoUsada();
}

QString Dfe_service::getUltNsuResumo(){
    return dfeRepo.getUltNsuResumo();
}

QString Dfe_service::getUltNsuXml(){
    return dfeRepo.getUltNsuXml();
}

bool Dfe_service::salvarNovoUltNsuXml(const QString &ultnsuxml){
    return dfeRepo.salvarNovoUltNsuXml(ultnsuxml);

}

bool Dfe_service::salvarNovoUltNsuResumo(const QString &ultnsu){
    return dfeRepo.salvarNovoUltNsuResumo(ultnsu);

}
bool Dfe_service::atualizarDataNsu(TipoDfeInfo tipo){
    return dfeRepo.atualizarDataNsu(tipo);
}

bool Dfe_service::possoConsultar(){
    QString maxData = dfeRepo.getMaxData();

    //-1 = erro
    if(maxData != "-1"){
        QDateTime dataMod = QDateTime::fromString(maxData, "yyyy-MM-dd HH:mm:ss");

        if (!dataMod.isValid()) {
            qDebug() << "Data inválida em dfe_info:" << maxData;
            return true; // se inválida, permite consultar
        }

        QDateTime agora = QDateTime::currentDateTime();

        qint64 diff = dataMod.secsTo(agora);

        qDebug() << "Diferença em segundos desde última consulta:" << diff;

        // 1 hora = 3600s
        return diff >= 3600;

    }else{
        return false;
    }


}

