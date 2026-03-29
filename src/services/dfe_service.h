#ifndef DFE_SERVICE_H
#define DFE_SERVICE_H

#include <QObject>
#include "../repository/dfeinfo_repository.h"


class Dfe_service : public QObject
{
    Q_OBJECT
public:
    explicit Dfe_service(QObject *parent = nullptr);
    QString getUltimaIdentificaçãoUsada();
    QString getUltNsuResumo();
    QString getUltNsuXml();
    bool salvarNovoUltNsuXml(const QString &ultnsuxml);
    bool salvarNovoUltNsuResumo(const QString &ultnsu);
    bool atualizarDataNsu(TipoDfeInfo tipo);
    bool possoConsultar();
private:
    DfeInfo_repository dfeRepo;

signals:
};

#endif // DFE_SERVICE_H
