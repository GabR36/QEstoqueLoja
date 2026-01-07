#include "DanfeUtil.h"
#include <QSqlQuery>
#include "../configuracao.h"
#include <QStandardPaths>
#include <QFileInfo>

DanfeUtil::DanfeUtil(QObject *parent)
    : QObject{parent}
{
    QStringList dataLocations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &basePath : dataLocations) {
        QString candidateNfe = basePath + "/QEstoqueLoja/reports/DANFE-NFe.xml";
        QString candidateNFCe = basePath + "/QEstoqueLoja/reports/DANFE-NFCe.xml";
        if (QFileInfo::exists(candidateNfe)) {
            caminhoReportNFe = candidateNfe;
            caminhoReportNFCe = candidateNFCe;
            break;
        }
    }

    //caminhoReportNFe = QStandardPaths::AppConfigLocation(QStandardPaths::GenericDataLocation) + "/reports/DANFE-NFe.xml";
    //caminhoReportNFCe = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/reports/DANFE-NFCe.xml";
    empresaValues = Configuracao::get_All_Empresa_Values();
    QString caminhoCompletoLogo = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                              "/imagens/" + QFileInfo(empresaValues.value("caminho_logo_empresa")).fileName();
    caminhoLogo = caminhoCompletoLogo;
}
bool DanfeUtil::abrirDanfe(int idVenda){
    if(!db.open()){
        qDebug() << "nao abriu banco danfeUtil";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT xml_path FROM notas_fiscais WHERE id_venda = :idvenda");
    query.bindValue(":idvenda", idVenda);

    if (!query.exec() || !query.next()) {
        return false;
    }

    QString xmlPath = query.value(0).toString();
    if (xmlPath.isEmpty()) {
        return false;
    }
    auto nf = AcbrManager::instance()->nfe();
    nf->LimparLista();
    nf->CarregarXML(xmlPath.toStdString());

    imprimirDanfe(nf);
    return true;
}
void DanfeUtil::imprimirDanfe(const ACBrNFe *nf){
    nf->Imprimir("", 1, "", true, true, std::nullopt, std::nullopt);
}
void DanfeUtil::setCaminhoLogo(QString logo){
    caminhoLogo = logo;
}

bool DanfeUtil::abrirDanfePorXml(const QString& xmlPath)
{
    if (xmlPath.isEmpty()) {
        qDebug() << "XML vazio";
        return false;
    }

    if (!QFileInfo::exists(xmlPath)) {
        qDebug() << "XML nao encontrado:" << xmlPath;
        return false;
    }

    auto nf = AcbrManager::instance()->nfe();
    nf->LimparLista();

    nf->CarregarXML(xmlPath.toStdString());


    imprimirDanfe(nf);
    return true;
}


