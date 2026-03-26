#include "pdfexporter.h"
#include <QChartView>
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>
#include <QSqlQuery>
#include <QDesktopServices>
#include <QLocale>
#include <QUrl>
#include <QFont>
#include <QDate>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QAbstractBarSeries>
#include <QBarSet>
#include <QAbstractAxis>
#include <QValueAxis>
#include <QColor>

PDFexporter::PDFexporter() {


}

void PDFexporter::exportarResumoContadorPdf(const QString &filePath,
                                             QDateTime dtIni, QDateTime dtFim,
                                             const QList<NotaFiscalDTO> &notas)
{
    QPdfWriter pdf(filePath);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);

    QPainter painter(&pdf);

    const int margemEsquerda   = 20;
    const int margemTopo       = 120;
    const int espacamentoLinha = 60;
    int y = margemTopo;

    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.drawText(margemEsquerda, y,
                     QString("NF Autorizadas no período de %1 até %2")
                         .arg(dtIni.date().toString("dd/MM/yyyy"))
                         .arg(dtFim.date().toString("dd/MM/yyyy")));
    y += 80;

    painter.setFont(QFont("Arial", 11, QFont::Bold));
    painter.drawText(margemEsquerda,        y, "Número");
    painter.drawText(margemEsquerda + 250,  y, "Emissão");
    painter.drawText(margemEsquerda + 500,  y, "Chave");
    painter.drawText(margemEsquerda + 1700, y, "Valor Total");
    painter.drawText(margemEsquerda + 2000, y, "Situação");
    y += espacamentoLinha;

    painter.setFont(QFont("Arial", 11));

    int    totalRegistros = 0;
    double totalGeral     = 0.0;
    QLocale ptBR(QLocale::Portuguese, QLocale::Brazil);

    for (const NotaFiscalDTO &nota : notas) {
        if (y + espacamentoLinha > pdf.height() - margemTopo) {
            pdf.newPage();
            y = margemTopo;
        }

        int    cstat          = nota.cstat.toInt();
        double valorParaTotal = nota.valorTotal;
        QString situacao;

        if (cstat == 135) {
            situacao       = "CANCELADO";
            valorParaTotal = 0;
        } else if (nota.finalidade == "DEVOLUCAO") {
            situacao       = "DEVOLUÇÃO";
            valorParaTotal = -nota.valorTotal;
        } else {
            situacao = "AUTORIZADA";
        }

        QDate dataEmissao = QDateTime::fromString(nota.dhEmi, "yyyy-MM-dd HH:mm:ss").date();

        painter.drawText(margemEsquerda,        y, QString::number(nota.nnf));
        painter.drawText(margemEsquerda + 250,  y, dataEmissao.toString("dd/MM/yyyy"));
        painter.drawText(margemEsquerda + 500,  y, nota.chNfe);
        painter.drawText(margemEsquerda + 1700, y, ptBR.toCurrencyString(valorParaTotal));
        painter.drawText(margemEsquerda + 2000, y, situacao);
        painter.drawLine(margemEsquerda, y + 15, pdf.width() - margemEsquerda, y + 15);

        totalRegistros++;
        totalGeral += valorParaTotal;
        y += espacamentoLinha;
    }

    y += 60;
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(margemEsquerda, y,
                     QString("Total de Registros: %1").arg(totalRegistros));
    y += 40;
    painter.drawText(margemEsquerda, y,
                     QString("Valor Total Geral: %1").arg(ptBR.toCurrencyString(totalGeral)));

    painter.end();
}

void PDFexporter::exportarGraficoRelatorio(QWidget *parent, QChartView *chartView)
{
    if (!chartView) {
        QMessageBox::information(parent, "Nada para exportar",
            "Aplique o filtro antes de exportar.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        parent,
        "Salvar PDF",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/relatorio.pdf",
        "PDF (*.pdf)");
    if (filePath.isEmpty()) return;

    QChart *chart = chartView->chart();

    // Habilitar rótulos nas barras com cor escura legível
    for (QAbstractSeries *series : chart->series()) {
        auto *barSeries = qobject_cast<QAbstractBarSeries*>(series);
        if (!barSeries) continue;
        barSeries->setLabelsVisible(true);
        barSeries->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);
        barSeries->setLabelsFormat("@value");
        for (QBarSet *barSet : barSeries->barSets())
            barSet->setLabelColor(QColor(30, 30, 30));
    }

    // Ampliar eixo Y 15% para acomodar rótulos acima das barras
    double originalMaxY = -1;
    for (QAbstractAxis *axis : chart->axes(Qt::Vertical)) {
        auto *vAxis = qobject_cast<QValueAxis*>(axis);
        if (vAxis) {
            originalMaxY = vAxis->max();
            vAxis->setMax(originalMaxY * 1.15);
            break;
        }
    }

    // Renderizar
    QPdfWriter pdf(filePath);
    pdf.setPageLayout(QPageLayout(
        QPageSize(QPageSize::A4),
        QPageLayout::Landscape,
        QMarginsF(10, 10, 10, 10),
        QPageLayout::Millimeter));
    pdf.setResolution(150);

    QPainter painter(&pdf);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(QRect(0, 0, pdf.width(), pdf.height()), Qt::white);

    QGraphicsScene *scene = chartView->scene();
    scene->render(&painter,
                  QRectF(0, 0, pdf.width(), pdf.height()),
                  scene->sceneRect());
    painter.end();

    // Restaurar estado original do gráfico na tela
    for (QAbstractSeries *series : chart->series()) {
        auto *barSeries = qobject_cast<QAbstractBarSeries*>(series);
        if (barSeries) barSeries->setLabelsVisible(false);
    }
    if (originalMaxY > 0) {
        for (QAbstractAxis *axis : chart->axes(Qt::Vertical)) {
            auto *vAxis = qobject_cast<QValueAxis*>(axis);
            if (vAxis) { vAxis->setMax(originalMaxY); break; }
        }
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

