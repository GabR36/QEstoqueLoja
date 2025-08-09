#include "ibptutil.h"
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>

IbptUtil::IbptUtil(QObject *parent)
    : QObject{parent}
{
    caminhoArquivoTabela = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/recursos/TabelaIBPTaxPR25.1.F.csv";
    tabela.setFileName(caminhoArquivoTabela);

    if (!tabela.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Não foi possível abrir o arquivo:" << caminhoArquivoTabela;
    }



}

float IbptUtil::get_Aliquota_From_Csv(QString ncm){

    if (!tabela.isOpen()) {
        qDebug() << "Arquivo não está aberto.";
        return false;
    }

    // Volta para o início do arquivo
    if (!tabela.seek(0)) {
        qDebug() << "Erro ao voltar para o início do arquivo.";
        return false;
    }

    QTextStream in(&tabela);
   // in.setCodec("UTF-8");  // ou "Latin1", conforme o encoding do CSV

    QString header = in.readLine();  // pula cabeçalho
    while (!in.atEnd()) {
        QString linha = in.readLine();
        QStringList campos = linha.split(';');

        if (campos.size() < 13)
            continue;

        QString ncmCSV = campos[0].trimmed();
        QString exCSV = campos[1].trimmed();

        if (ncmCSV == ncm && exCSV == "") {
            bool ok1, ok2, ok3;
            double nacional = campos[4].replace(',', '.').toDouble(&ok1);  // nacionalfederal
            double estadual = campos[6].replace(',', '.').toDouble(&ok2);  // estadual
            double municipal = campos[7].replace(',', '.').toDouble(&ok3); // municipal

            if (ok1 && ok2 && ok3)
                return nacional + estadual + municipal;
        }
    }

    return -1.0;  // não encontrado
}

QStringList IbptUtil::get_Sugestoes_NCM(QString filtro) {
    // QFile file(caminhoArquivoTabela);
    QStringList sugestoes;


    QTextStream in(&tabela);
    in.readLine(); // pula cabeçalho: codigo;ex;tipo;descricao;

    while (!in.atEnd()) {
        QString linha = in.readLine();
        QStringList campos = linha.split(';');
        if (campos.size() < 4)
            continue;

        QString codigo = campos[0].trimmed();
        QString descricao = campos[3].trimmed();
        QString sugestao = codigo + " - " + descricao;

        if (filtro.isEmpty() || sugestao.contains(filtro, Qt::CaseInsensitive))
            sugestoes << sugestao;
    }
    // qDebug() << "Total de sugestões carregadas:" << sugestoes.size();
    // for (const QString &sugestao : sugestoes)
    //     qDebug() << sugestao;

    return sugestoes;
}

bool IbptUtil::eh_Valido_NCM(QString ncm){
    if (!tabela.isOpen()) {
        qDebug() << "Arquivo não está aberto.";
        return false;
    }

    // Volta para o início do arquivo
    if (!tabela.seek(0)) {
        qDebug() << "Erro ao voltar para o início do arquivo.";
        return false;
    }

    QTextStream in(&tabela);
    in.readLine();  // pula o cabeçalho

    while (!in.atEnd()) {
        QString linha = in.readLine();
        QStringList campos = linha.split(';');
        if (campos.size() < 2)
            continue;

        QString ncmCSV = campos[0].trimmed();
        QString exCSV = campos[1].trimmed();

        if (ncmCSV == ncm && exCSV == "")
            return true;
    }

    return false;
}
QString IbptUtil::get_Descricao_NCM(QString ncm) {
    if (!tabela.isOpen()) {
        qDebug() << "Arquivo não está aberto.";
        return "";
    }

    if (!tabela.seek(0)) {
        qDebug() << "Erro ao voltar para o início do arquivo.";
        return "";
    }

    QTextStream in(&tabela);
    in.readLine();  // pula o cabeçalho

    while (!in.atEnd()) {
        QString linha = in.readLine();
        QStringList campos = linha.split(';');

        if (campos.size() < 4)
            continue;

        QString ncmCSV = campos[0].trimmed();
        QString exCSV = campos[1].trimmed();
        QString descricao = campos[3].trimmed();

        if (ncmCSV == ncm && exCSV == "") {
            return descricao;
        }
    }

    return "";  // não encontrado
}





