#include "ibptutil.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>

IbptUtil::IbptUtil(QObject *parent)
    : QObject{parent}
{}

float IbptUtil::get_Aliquota_From_Csv(QString ncm){
    QString caminhoArquivo = QCoreApplication::applicationDirPath() + "/recursos/TabelaIBPTaxPR25.1.F.csv";
    QFile file(caminhoArquivo);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Não foi possível abrir o arquivo:" << caminhoArquivo;
        return -1.0;
    }

    QTextStream in(&file);
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
    QFile file(caminhoArquivoTabela);
    QStringList sugestoes;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Não foi possível abrir o arquivo:" << caminhoArquivoTabela;
        return sugestoes;
    }

    QTextStream in(&file);
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




