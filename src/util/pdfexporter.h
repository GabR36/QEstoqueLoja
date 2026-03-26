#ifndef PDFEXPORTER_H
#define PDFEXPORTER_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QSqlDatabase>
#include <QWidget>
#include "../dto/NotaFiscal_dto.h"

class QChartView;

class PDFexporter
{
public:
    PDFexporter();

    static void exportarResumoContadorPdf(const QString &filePath,
                                          QDateTime dtIni, QDateTime dtFim,
                                          const QList<NotaFiscalDTO> &notas);

    // Exporta o gráfico atual para PDF (A4 landscape).
    // Exibe QFileDialog, mostra valores nas barras e abre o arquivo após salvar.
    static void exportarGraficoRelatorio(QWidget *parent, QChartView *chartView);

private:
   // QSqlDatabase db = QSqlDatabase::database();

};

#endif // PDFEXPORTER_H
