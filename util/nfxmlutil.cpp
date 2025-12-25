#include "nfxmlutil.h"
#include <QFile>
#include <QDomDocument>
#include <QDebug>

NfXmlUtil::NfXmlUtil(QObject *parent)
    : QObject{parent}
{}

double NfXmlUtil::getDouble(const QString &str) {
    return str.trimmed().replace(",", ".").toDouble();
}

// Busca tag ignorando namespace
QDomElement NfXmlUtil::findTag(QDomElement parent, const QString &tagName)
{
    for (QDomNode n = parent.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            if (e.tagName().endsWith(":" + tagName) || e.tagName() == tagName)
                return e;
        }
    }
    return QDomElement();
}

// Busca lista de tags ignorando namespace
QDomNodeList NfXmlUtil::findTags(QDomDocument &doc, const QString &tagName)
{
    QDomNodeList list = doc.elementsByTagName(tagName);
    if (!list.isEmpty()) return list;

    // Se não achou, faz busca completa
    QDomNodeList all = doc.elementsByTagName("*");
    QList<QDomNode> matched;

    for (int i = 0; i < all.size(); i++) {
        QDomElement e = all.at(i).toElement();
        if (e.tagName().endsWith(":" + tagName) || e.tagName() == tagName) {
            matched.append(all.at(i));
        }
    }

    QDomNodeList out;
    // construir uma lista nova (tecnicamente QDomNodeList não permite criar diretamente,
    // mas podemos retornar um QList<QDomNode> se preferir).
    return list; // fallback
}

CustoItem NfXmlUtil::calcularCustoItemSN(const QString &xmlPath, int nItem)
{
    CustoItem ci;

    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Erro ao abrir XML";
        return ci;
    }

    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QDomElement root = doc.documentElement();
    QDomElement nfe = root.firstChildElement("NFe");
    QDomElement infNFe = nfe.firstChildElement("infNFe");

    // --- TOTAL DA NOTA (para rateios)
    QDomElement total = infNFe.firstChildElement("total")
                            .firstChildElement("ICMSTot");

    double vNF      = total.firstChildElement("vNF").text().toDouble();
    double vFrete   = total.firstChildElement("vFrete").text().toDouble();
    double vSeg     = total.firstChildElement("vSeg").text().toDouble();
    double vOutros  = total.firstChildElement("vOutro").text().toDouble();
    double vDesc    = total.firstChildElement("vDesc").text().toDouble();
    double vProdTot = total.firstChildElement("vProd").text().toDouble();

    // --- ACHANDO O ITEM EXATO
    QDomNodeList itens = infNFe.elementsByTagName("det");
    QDomElement itemEncontrado;

    for (int i = 0; i < itens.count(); i++) {
        QDomElement det = itens.at(i).toElement();
        if (det.attribute("nItem").toInt() == nItem) {
            itemEncontrado = det;
            break;
        }
    }

    if (itemEncontrado.isNull()) {
        qWarning() << "Item não encontrado!";
        return ci;
    }

    // --- PROD
    QDomElement prod = itemEncontrado.firstChildElement("prod");
    double vProd = prod.firstChildElement("vProd").text().toDouble();
    double qCom  = prod.firstChildElement("qCom").text().toDouble();

    ci.vProd = vProd;
    ci.precoUnitarioNota = vProd / qCom;       // 57.27643 no seu XML

    // --- IPI (CST 50 → tributado)
    QDomElement imposto = itemEncontrado.firstChildElement("imposto");
    QDomElement ipi = imposto.firstChildElement("IPI");
    double vIPI = 0;

    if (!ipi.isNull()) {
        QDomElement ipiTrib = ipi.firstChildElement("IPITrib");
        if (!ipiTrib.isNull()) {
            vIPI = ipiTrib.firstChildElement("vIPI").text().toDouble();
        }
    }

    ci.vIPI = vIPI;

    double ipiUnit = (vIPI / qCom);  // ~3.724

    // --- RATEIOS (proporcionais ao valor do item)
    double proporcao = vProd / vProdTot;

    ci.rateioFrete   = proporcao * vFrete;
    ci.rateioSeguro  = proporcao * vSeg;
    ci.rateioOutros  = proporcao * vOutros;
    ci.rateioDesconto = proporcao * vDesc;

    double rateioUnit = 0;
    if (qCom > 0)
        rateioUnit = (ci.rateioFrete + ci.rateioSeguro +
                      ci.rateioOutros - ci.rateioDesconto) / qCom;

    // --- CUSTO UNITÁRIO FINAL
    ci.custoUnitario =
        ci.precoUnitarioNota   // base da nota
        + ipiUnit              // IPI rateado
        + rateioUnit;          // Frete/Seguro/Outros/Desc rateados

    ci.custoTotal = ci.custoUnitario * qCom;

    return ci;
}
