#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include <QWidget>
#include <QList>
#include <QStringList>

class CsvExporter
{
public:
    // linhas[0] = cabeçalho, linhas[1..] = dados. Separador: ';', encoding: UTF-8.
    // Exibe QFileDialog, abre o arquivo após salvar.
    // Retorna false se o usuário cancelou ou ocorreu erro de escrita.
    static bool exportar(QWidget *parent,
                         const QString &nomeArquivoPadrao,
                         const QList<QStringList> &linhas);
};

#endif // CSVEXPORTER_H
