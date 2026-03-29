#include "printutil.h"
#include <QMessageBox>
#include <QByteArray>
#include <zint.h>
#include <QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QPainter>
#include <QLocale>

PrintUtil::PrintUtil(QObject *parent)
    : QObject{parent}
{}

bool PrintUtil::imprimirEtiquetas(QWidget *parent,
                                     int quant,
                                     const QImage &barcodeImage,
                                     const QString &desc,
                                     double preco,
                                     QString *erro)
{
    const int margemEsquerda = 10;

    if (barcodeImage.isNull()) {
        if (erro) *erro = "Imagem de código de barras inválida";
        return false;
    }

    QPrinter printer;
    printer.setPageSize(QPageSize(QSizeF(80, 2000), QPageSize::Millimeter));

    QPrintDialog dialog(&printer, parent);
    if (dialog.exec() == QDialog::Rejected)
        return false;

    QPainter painter;
    if (!painter.begin(&printer)) {
        if (erro) *erro = "Falha ao iniciar impressão";
        return false;
    }

    QLocale portugues(QLocale::Portuguese, QLocale::Brazil);

    int ypos[2] = {5, 53};
    const int espacoEntreItens = 20;

    for (int i = 0; i < quant; i++) {

        if (i > 0) {
            for (int j = 0; j < 2; j++)
                ypos[j] += 51;
        }

        // descrição
        QRect descRect(margemEsquerda, ypos[0], 145, 32);
        QFont fonte = painter.font();
        fonte.setPointSize(10);
        fonte.setBold(false);
        painter.setFont(fonte);
        painter.drawText(descRect, Qt::TextWordWrap, desc);

        // preço
        fonte.setBold(true);
        painter.setFont(fonte);
        painter.drawText(margemEsquerda,
                         ypos[1],
                         "Preço: R$ " + portugues.toString(preco, 'f', 2));

        // código de barras
        QRect codImageRect(margemEsquerda + 140, ypos[0], 108, 50);
        painter.drawImage(codImageRect, barcodeImage);

        for (int j = 0; j < 2; j++)
            ypos[j] += espacoEntreItens;
    }

    painter.end();
    return true;
}

