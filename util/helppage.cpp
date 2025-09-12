#include "helppage.h"
#include "ui_helppage.h"
#include <QFileInfo>
#include <QStandardPaths>

HelpPage::HelpPage(QWidget *parent, QString idtopico)
    : QDialog(parent)
    , ui(new Ui::HelpPage)
{
    ui->setupUi(this);

    ui->TextB_Main->setOpenExternalLinks(true);

    QStringList dataLocations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    bool found = false;

    for (const QString &basePath : dataLocations) {
        QString candidate = basePath + "/QEstoqueLoja/recursos/ajuda/help.html";
        qDebug() << "Verificando:" << candidate;
        if (QFileInfo::exists(candidate)) {
            caminhoArquivoHtml = candidate;
            found = true;
            break;
        }
    }

    if (!found) {
        qWarning() << "Arquivo html não encontrado em nenhum dos caminhos padrão.";
    } else {
        qDebug() << "Arquivo encontrado em:" << caminhoArquivoHtml;
        QFile arquivo(caminhoArquivoHtml);
        if (!arquivo.open(QIODevice::ReadOnly)) {
            qWarning() << "Falha ao abrir o arquivo:" << arquivo.errorString();
        }
    }

    // Usa o parâmetro passado, se não vier nada, abre o índice
    QString basePath = caminhoArquivoHtml;
    QString url = "file://" + basePath;

    if (!idtopico.isEmpty()) {
        url += "#" + idtopico;
    }

    ui->TextB_Main->setSource(QUrl(url));

    setWindowTitle("Ajuda do Sistema - QEstoqueLoja");
    resize(900, 600);
}

HelpPage::~HelpPage()
{
    delete ui;
}
