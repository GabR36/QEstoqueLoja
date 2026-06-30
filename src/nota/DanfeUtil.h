#ifndef DANFEUTIL_H
#define DANFEUTIL_H
#include <QObject>
#include <QSqlDatabase>
#include <QLocale>
#include <QMap>
#include "../nota/acbrmanager.h"
#include "../services/config_service.h"
#include "../infra/apppath_service.h"
#include "../services/notafiscal_service.h"

class DanfeUtil : public QObject
{
    Q_OBJECT
public:
    explicit DanfeUtil(QObject *parent = nullptr);
    bool abrirDanfe(qlonglong idVenda);
    void imprimirDanfe(const ACBrNFe *nf);
    void setCaminhoLogo(QString logo);
    bool abrirDanfePorXml(const QString& xmlPath);

    bool abrirDanfePorXmlEvento(const QString &xmlPath);
private:
    QSqlDatabase db = QSqlDatabase::database();
    QLocale portugues;
    QString caminhoReportNFe,caminhoReportNFCe,caminhoLogo;
    QMap<QString,QString> empresaValues;
    ConfigDTO configDTO;
    NotaFiscal_service notaServ;
signals:
};

#endif // DANFEUTIL_H
