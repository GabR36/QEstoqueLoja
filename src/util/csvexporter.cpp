#include "csvexporter.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

bool CsvExporter::exportar(QWidget *parent,
                            const QString &nomeArquivoPadrao,
                            const QList<QStringList> &linhas)
{
    if (linhas.isEmpty()) {
        QMessageBox::information(parent, "Sem dados", "Não há dados para exportar.");
        return false;
    }

    QString filePath = QFileDialog::getSaveFileName(
        parent,
        "Salvar CSV",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + "/" + nomeArquivoPadrao,
        "CSV (*.csv)");
    if (filePath.isEmpty())
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(parent, "Erro", "Não foi possível criar o arquivo:\n" + filePath);
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    auto quotarCampo = [](const QString &campo) -> QString {
        QString c = campo;
        c.replace('"', "\"\"");
        return '"' + c + '"';
    };

    for (const QStringList &linha : linhas) {
        QStringList campos;
        for (const QString &campo : linha)
            campos << quotarCampo(campo);
        out << campos.join(';') << "\n";
    }

    file.close();
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    return true;
}
