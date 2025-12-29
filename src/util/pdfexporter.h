#ifndef PDFEXPORTER_H
#define PDFEXPORTER_H

#include <QString>
#include <QSqlDatabase>

class PDFexporter
{
public:
    PDFexporter();
    static void exportarTodosProdutosParaPDF(const QString &fileName);
    static void exportarNfProdutosParaPDF(const QString &fileName);

private:
   // QSqlDatabase db = QSqlDatabase::database();

};

#endif // PDFEXPORTER_H
