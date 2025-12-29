#ifndef NFXMLUTIL_H
#define NFXMLUTIL_H

#include <QObject>
#include <QDomElement>


struct CustoItem {
    double custoUnitario = 0;
    double custoTotal = 0;

    double precoUnitarioNota = 0;
    double vProd = 0;
    double vIPI = 0;
    double vICMSST = 0;

    double rateioFrete = 0;
    double rateioSeguro = 0;
    double rateioOutros = 0;
    double rateioDesconto = 0;
};

class NfXmlUtil : public QObject
{
    Q_OBJECT
public:
    explicit NfXmlUtil(QObject *parent = nullptr);

    static double getDouble(const QString &str);
    CustoItem calcularCustoItemSN(const QString &xmlPath, int nItem);
private:
    QDomElement findTag(QDomElement parent, const QString &tagName);

    QDomNodeList findTags(QDomDocument &doc, const QString &tagName);
};

#endif // NFXMLUTIL_H
