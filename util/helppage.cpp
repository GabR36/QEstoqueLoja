#include "helppage.h"
#include "ui_helppage.h"
#include <QStandardPaths>
#include <QFileInfo>

HelpPage::HelpPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HelpPage)
{
    ui->setupUi(this);
    ui->TextB_Main->setOpenExternalLinks(true);


    QStringList dataLocations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    bool found = false;
    for (const QString &basePath : dataLocations) {
        QString candidate = basePath + "/QEstoqueLoja/recursos/HelpPage/help.html";
        qDebug() << "Verificando:" << candidate;
        if (QFileInfo::exists(candidate)) {
            caminhoArquivoHtml = candidate;
            found = true;
            break;
        }
    }

    if (!found) {
        qWarning() << "Arquivo html não encontrado em nenhum dos caminhos padrão.";
    }else{
        ui->TextB_Main->setSource(QUrl::fromLocalFile(caminhoArquivoHtml));
    }

    setWindowTitle("Ajuda");
}

HelpPage::~HelpPage()
{
    delete ui;
}
