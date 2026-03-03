#include "nfxmlutil.h"
#include <QDebug>
#include <QFile>
#include <QDomDocument>
#include <QDomNode>
#include <QSqlError>
#include <QDomElement>

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


NotaFiscalDTO NfXmlUtil::lerNotaFiscalDoXML(const QString &xmlPath)
{
    NotaFiscalDTO nf;
    nf.xmlPath = xmlPath;

    QFile file(xmlPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Erro ao abrir XML:" << xmlPath;
        return nf;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Erro ao carregar XML:" << xmlPath;
        file.close();
        return nf;
    }
    file.close();

    auto getTag = [&](const QDomElement &parent, const QString &tag) {
        QDomNode n = parent.elementsByTagName(tag).item(0);
        return n.isNull() ? QString() : n.toElement().text().trimmed();
    };

    // Pegando nó nfeProc ou NFe
    QDomNodeList listaInfNFe = doc.elementsByTagName("infNFe");
    if (listaInfNFe.isEmpty()) {
        qWarning() << "XML sem <infNFe>";
        return nf;
    }

    QDomElement infNFe = listaInfNFe.at(0).toElement();

    // CHAVE DA NFE (atributo Id removendo "NFe")
    QString id = infNFe.attribute("Id");
    if (id.startsWith("NFe"))
        nf.chNfe = id.mid(3);

    // --- TAGS DIRETAS DO INFNFE ---
    QDomNodeList ideList = infNFe.elementsByTagName("ide");
    if (!ideList.isEmpty()) {
        QDomElement ide = ideList.at(0).toElement();
        nf.cuf   = getTag(ide, "cUF");
        nf.nnf   = getTag(ide, "nNF").toLongLong();
        nf.serie = getTag(ide, "serie").toInt();
        nf.modelo = getTag(ide, "mod");
        nf.tpAmb = (getTag(ide, "tpAmb") == "1");  // 1 = produção
        nf.dhEmi = getTag(ide, "dhEmi");
    }

    // --- VALOR TOTAL DA NOTA ---
    QDomNodeList totalList = infNFe.elementsByTagName("ICMSTot");
    if (!totalList.isEmpty()) {
        QDomElement tot = totalList.at(0).toElement();
        nf.valorTotal = getTag(tot, "vNF").toDouble();
    }

    // --- EMITENTE ---
    QDomNodeList emitList = infNFe.elementsByTagName("emit");
    if (!emitList.isEmpty()) {
        QDomElement emite = emitList.at(0).toElement();
        nf.cnpjEmit = getTag(emite, "CNPJ");
    }

    // --- <protNFe> (protocolo) ---
    QDomNodeList protList = doc.elementsByTagName("protNFe");
    if (!protList.isEmpty()) {
        QDomElement prot = protList.at(0).toElement();
        QDomElement infProt = prot.elementsByTagName("infProt").item(0).toElement();

        nf.cstat = getTag(infProt, "cStat");
        nf.nProt = getTag(infProt, "nProt");
    }

    return nf;
}


QList<ProdutoNotaDTO> NfXmlUtil::carregarProdutosDaNFe(const QString &xml_path, qlonglong id_nf)
{
    QList<ProdutoNotaDTO> lista;

    QFile file(xml_path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Erro ao abrir arquivo XML:" << xml_path;
        return lista;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qDebug() << "Erro ao ler conteúdo do XML";
        file.close();
        return lista;
    }
    file.close();

    QDomNodeList itens = doc.elementsByTagName("det");

    for (int i = 0; i < itens.count(); i++) {
        QDomElement det = itens.at(i).toElement();

        ProdutoNotaDTO p;
        p.idNf = id_nf;

        p.nitem = det.attribute("nItem").toInt();

        QDomElement prod = det.firstChildElement("prod");

        p.descricao          = prod.firstChildElement("xProd").text();
        p.codigoBarras    = prod.firstChildElement("cEAN").text();
        p.uCom  = prod.firstChildElement("uCom").text();
        p.ncm           = prod.firstChildElement("NCM").text();
        p.cfop          = prod.firstChildElement("CFOP").text();
        p.quantidade         = prod.firstChildElement("qCom").text().replace(",", ".").toDouble();
        p.preco         = prod.firstChildElement("vUnCom").text().replace(",", ".").toDouble();

        // ======= IMPOSTOS =======
        QDomElement imposto = det.firstChildElement("imposto");

        // -------------------- CSOSN ou CST ICMS --------------------
        p.csosn = "";
        p.cstIcms = "";
        p.temSt = false;

        QDomElement icms = imposto.firstChildElement("ICMS");

        QStringList gruposICMS = {
            "ICMS00","ICMS10","ICMS20","ICMS30","ICMS40","ICMS51","ICMS60","ICMS70","ICMS90",
            "ICMSSN101","ICMSSN102","ICMSSN201","ICMSSN202","ICMSSN500","ICMSSN900"
        };

        for (const QString &g : gruposICMS) {
            QDomElement e = icms.firstChildElement(g);
            if (!e.isNull()) {

                // pega CST ou CSOSN
                QString CST = e.firstChildElement("CST").text();
                QString CSOSN = e.firstChildElement("CSOSN").text();

                if (!CSOSN.isEmpty())
                    p.csosn = CSOSN;

                if (!CST.isEmpty())
                    p.cstIcms = CST;

                // identifica ST
                if (g == "ICMS10" || g == "ICMS30" || g == "ICMS60" ||
                    g == "ICMS70" || g == "ICMSSN201" || g == "ICMSSN202") {
                    p.temSt = true;
                }

                // valida ST por valores
                if (!e.firstChildElement("vBCST").isNull() ||
                    !e.firstChildElement("vICMSST").isNull() ||
                    !e.firstChildElement("vBCSTRet").isNull() ||
                    !e.firstChildElement("vICMSSTRet").isNull())
                {
                    p.temSt = true;
                }

                break;
            }
        }

        // -------------------- PIS --------------------
        QDomNodeList listaPIS = imposto.elementsByTagName("PISOutr");
        if (listaPIS.isEmpty())
            listaPIS = imposto.elementsByTagName("PISNT");

        if (!listaPIS.isEmpty())
            p.pis = listaPIS.at(0).firstChildElement("CST").text();
        else
            p.pis = "";

        QString aliquota = listaPIS.at(0).firstChildElement("pPIS").text();
        p.aliquotaIcms = aliquota.replace(",", ".").toDouble();

        lista.append(p);
    }

    return lista;
}


ClienteDTO NfXmlUtil::getEmitenteFromXML(const QString &xmlPath) {
    ClienteDTO e;
    QFile file(xmlPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Erro ao abrir XML:" << xmlPath;
        return e;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Erro ao ler XML:" << xmlPath;
        return e;
    }

    file.close();

    auto getTag = [&](const QDomElement &parent, const QString &tag) {
        QDomNode n = parent.elementsByTagName(tag).item(0);
        return n.isNull() ? QString() : n.toElement().text().trimmed();
    };

    // Posiciona no nó <emit>
    QDomNodeList emits = doc.elementsByTagName("emit");
    if (emits.isEmpty()) {
        qWarning() << "XML sem <emit>";
        return e;
    }

    QDomElement emitEl = emits.at(0).toElement();

    // Dados diretos
    e.cpf = getTag(emitEl, "CNPJ");
    e.nome = getTag(emitEl, "xNome");
    e.ie   = getTag(emitEl, "IE");

    // Endereço
    QDomNode endNode = emitEl.elementsByTagName("enderEmit").item(0);
    if (!endNode.isNull()) {
        QDomElement end = endNode.toElement();
        e.endereco   = getTag(end, "xLgr");
        e.numeroEnd    = getTag(end, "nro").toLongLong();
        e.bairro = getTag(end, "xBairro");
        e.xMun   = getTag(end, "xMun");
        e.cMun   = getTag(end, "cMun");
        e.uf     = getTag(end, "UF");
        e.cep    = getTag(end, "CEP");
    }

    return e;
}
