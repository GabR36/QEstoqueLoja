#ifndef IBPTUTIL_H
#define IBPTUTIL_H

#include <QObject>
#include <QCoreApplication>

class IbptUtil : public QObject
{
    Q_OBJECT
public:
    explicit IbptUtil(QObject *parent = nullptr);

    static float get_Aliquota_From_Csv(QString ncm);
    QStringList get_Sugestoes_NCM(QString filtro = "");
private:
    QString caminhoArquivoTabela = QCoreApplication::applicationDirPath() + "/recursos/TabelaIBPTaxPR25.1.F.csv";

signals:
};

#endif // IBPTUTIL_H
