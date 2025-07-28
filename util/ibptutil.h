#ifndef IBPTUTIL_H
#define IBPTUTIL_H

#include <QObject>
#include <QCoreApplication>
#include <QFile>

class IbptUtil : public QObject
{
    Q_OBJECT
public:
    explicit IbptUtil(QObject *parent = nullptr);

    float get_Aliquota_From_Csv(QString ncm);
    QStringList get_Sugestoes_NCM(QString filtro = "");
    bool eh_Valido_NCM(QString ncm);
    QString get_Descricao_NCM(QString ncm);
private:
    QString caminhoArquivoTabela;
    QFile tabela;

signals:
};

#endif // IBPTUTIL_H
