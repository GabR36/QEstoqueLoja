#include "consultacnpj_service.h"
#include <QDebug>
#include <QRegularExpression>

ConsultaCnpj_service::ConsultaCnpj_service(QObject *parent)
    : QObject{parent}
{
    cnpjManager = ConsultaCnpjManager::instance()->cnpj();

}

ClienteDTO ConsultaCnpj_service::getInfo(QString cnpj){
    ClienteDTO info;
    QString retorno;
    try {
        std::string resultado = cnpjManager->Consultar(cnpj.toStdString());
        retorno = QString::fromStdString(resultado);
        qDebug() << "Consulta CNPJ retornou:" << retorno.left(200);
    } catch (const std::exception &e) {
        qDebug() << "Erro na consulta CNPJ:" << e.what();
        return info;
    }

    QStringList linhas = retorno.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);

    for (const QString &linha : linhas) {
        if (linha.startsWith("RazaoSocial="))
            info.nome = linha.section('=', 1).trimmed();
        else if (linha.startsWith("Endereco="))
            info.endereco = linha.section('=', 1).trimmed();
        else if (linha.startsWith("Numero="))
            info.numeroEnd = linha.section('=', 1).trimmed().toLongLong();
        else if (linha.startsWith("Bairro="))
            info.bairro = linha.section('=', 1).trimmed();
        else if (linha.startsWith("Cidade="))
            info.xMun = linha.section('=', 1).trimmed();
        else if (linha.startsWith("Abertura="))
            info.dataNasc = linha.section('=', 1).trimmed();
        else if (linha.startsWith("CEP="))
            info.cep = linha.section('=', 1).trimmed();
        else if (linha.startsWith("UF="))
            info.uf = linha.section('=', 1).trimmed();
        else if (linha.startsWith("InscricaoEstadual="))
            info.ie = linha.section('=', 1).trimmed();
    }

    //se tem IE, indicador ie dest = contribuinte icms
    if(!info.ie.isEmpty()){
        info.indIeDest = 1; // 1 = contribuinte
    }else{
        info.indIeDest = 0; // 0 nao contribuinte
    }
    return info;
}
