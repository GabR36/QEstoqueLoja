#ifndef DFEINFO_REPOSITORY_H
#define DFEINFO_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>

enum class TipoDfeInfo {
    ConsultaResumo,
    ConsultaXml,
};

class DfeInfo_repository : public QObject
{
    Q_OBJECT
public:
    explicit DfeInfo_repository(QObject *parent = nullptr);
    QSqlDatabase m_db;

    QString getUltimaIdentificaçãoUsada();
    QString getUltNsuResumo();
    QString getUltNsuXml();
    QString getDataAgora();
    bool salvarNovoUltNsuXml(const QString &ultnsuxml);
    bool salvarNovoUltNsuResumo(const QString &ultNsu);
    bool atualizarDataNsu(TipoDfeInfo tipo);
    QString getMaxData();
signals:
};

#endif // DFEINFO_REPOSITORY_H
