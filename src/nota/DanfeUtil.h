#ifndef DANFEUTIL_H
#define DANFEUTIL_H
#include <QObject>
#include <QSqlDatabase>
#include <QLocale>
#include <QMap>
#include "../nota/acbrmanager.h"


class DanfeUtil : public QObject
{
    Q_OBJECT
public:
    explicit DanfeUtil(QObject *parent = nullptr);
    bool abrirDanfe(int idVenda);
    void imprimirDanfe(const ACBrNFe *nf);
    void setCaminhoLogo(QString logo);
    bool abrirDanfePorXml(const QString& xmlPath);

    bool abrirDanfePorXmlEvento(const QString &xmlPath);
private:
    QSqlDatabase db = QSqlDatabase::database();
    QLocale portugues;
    QString caminhoReportNFe,caminhoReportNFCe,caminhoLogo;
    QMap<QString,QString> empresaValues;
signals:
};

#endif // DANFEUTIL_H
