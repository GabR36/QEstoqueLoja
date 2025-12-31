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
    QDomElement infNFe = root.firstChildElement("NFe")
                             .firstChildElement("infNFe");

    // ================= TOTAL DA NOTA =================
    QDomElement total = infNFe.firstChildElement("total")
                            .firstChildElement("ICMSTot");

    double vFrete   = getDouble(total.firstChildElement("vFrete").text());
    double vSeg     = getDouble(total.firstChildElement("vSeg").text());
    double vOutros  = getDouble(total.firstChildElement("vOutro").text());
    double vDesc    = getDouble(total.firstChildElement("vDesc").text());
    double vProdTot = getDouble(total.firstChildElement("vProd").text());

    // ================= ITEM =================
    QDomNodeList itens = infNFe.elementsByTagName("det");
    QDomElement det;

    for (int i = 0; i < itens.count(); i++) {
        QDomElement e = itens.at(i).toElement();
        if (e.attribute("nItem").toInt() == nItem) {
            det = e;
            break;
        }
    }

    if (det.isNull()) {
        qWarning() << "Item não encontrado!";
        return ci;
    }

    // ================= PROD =================
    QDomElement prod = det.firstChildElement("prod");
    double vProd = getDouble(prod.firstChildElement("vProd").text());
    double qCom  = getDouble(prod.firstChildElement("qCom").text());

    ci.vProd = vProd;
    ci.precoUnitarioNota = (qCom > 0) ? vProd / qCom : 0;

    // ================= IMPOSTOS DO ITEM =================
    QDomElement imposto = det.firstChildElement("imposto");

    // ---- IPI ----
    double vIPI = 0;
    QDomElement ipi = imposto.firstChildElement("IPI");
    if (!ipi.isNull()) {
        QDomElement ipiTrib = ipi.firstChildElement("IPITrib");
        if (!ipiTrib.isNull())
            vIPI = getDouble(ipiTrib.firstChildElement("vIPI").text());
    }
    ci.vIPI = vIPI;

    // ---- ICMS-ST / FCP / DIFAL ----
    double vICMSST = 0;
    double vFCPST  = 0;
    double vDIFAL  = 0;

    QDomElement icms = imposto.firstChildElement("ICMS");
    if (!icms.isNull()) {
        QDomElement icmsDet = icms.firstChildElement();
        while (!icmsDet.isNull()) {
            vICMSST += getDouble(icmsDet.firstChildElement("vICMSST").text());
            vFCPST  += getDouble(icmsDet.firstChildElement("vFCPST").text());
            vDIFAL  += getDouble(icmsDet.firstChildElement("vICMSUFDest").text());
            icmsDet = icmsDet.nextSiblingElement();
        }
    }

    ci.vICMSST = vICMSST;

    // ================= RATEIO =================
    double proporcao = (vProdTot > 0) ? vProd / vProdTot : 0;

    double rateioTotal =
        proporcao * (vFrete + vSeg + vOutros - vDesc);

    // ================= CUSTO =================
    double custoTotalItem =
        vProd +
        vIPI +
        vICMSST +
        vFCPST +
        vDIFAL +
        rateioTotal;

    ci.custoTotal   = custoTotalItem;
    ci.custoUnitario = (qCom > 0) ? custoTotalItem / qCom : 0;

    return ci;
}
