#include "DanfeUtil.h"
#include <QSqlQuery>
#include "../configuracao.h"
#include <QStandardPaths>

DanfeUtil::DanfeUtil(QObject *parent)
    : QObject{parent}
{
    caminhoReportNFe = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/reports/DANFE-NFe.xml";
    caminhoReportNFCe = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/reports/DANFE-NFCe.xml";
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

    // Usar smart pointer para gerenciar a memória
    QScopedPointer<CppNFe> _nfe(new CppNFe);
    if(!_nfe->notafiscal->loadFromFile(xmlPath)){
        qDebug() << "erro ao load from file";
        return false;
    }

    imprimirDanfe(_nfe.data()); // Passa o ponteiro bruto (safe porque ainda está no escopo)
    return true;
}
void DanfeUtil::imprimirDanfe(const CppNFe *cppnfe){

    CppDanfeQtRPT danfe(cppnfe, 0);

    danfe.caminhoLogo(caminhoLogo);
    if (cppnfe->notafiscal->retorno->protNFe->items->count() > 0)
    {
        if ((cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat() == 100) ||
            (cppnfe->notafiscal->retorno->protNFe->items->value(0)->get_cStat() == 150))
        {
            if (cppnfe->notafiscal->NFe->items->value(0)->infNFe->ide->get_mod() == ModeloDF::NFe)
                danfe.caminhoArquivo(caminhoReportNFe);
            else
                danfe.caminhoArquivo(caminhoReportNFCe);

            danfe.print();
        }
    } else
    {
        if (cppnfe->notafiscal->NFe->items->value(0)->infNFe->ide->get_mod() == ModeloDF::NFe)
            danfe.caminhoArquivo(caminhoReportNFe);
        else
            danfe.caminhoArquivo(caminhoReportNFCe);

        danfe.print();
    }
}
void DanfeUtil::setCaminhoLogo(QString logo){
    caminhoLogo = logo;
}


