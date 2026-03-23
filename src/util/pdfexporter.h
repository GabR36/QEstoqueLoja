#ifndef PDFEXPORTER_H
#define PDFEXPORTER_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QSqlDatabase>
#include "../dto/NotaFiscal_dto.h"

class PDFexporter
{
public:
    PDFexporter();
    static void exportarTodosProdutosParaPDF(const QString &fileName);
    static void exportarNfProdutosParaPDF(const QString &fileName);
    static void exportarResumoContadorPdf(const QString &filePath,
                                          QDateTime dtIni, QDateTime dtFim,
                                          const QList<NotaFiscalDTO> &notas);

private:
   // QSqlDatabase db = QSqlDatabase::database();

};

#endif // PDFEXPORTER_H
