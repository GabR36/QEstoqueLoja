#ifndef HELPPAGE_H
#define HELPPAGE_H

#include <QDialog>


namespace Ui {
class HelpPage;
}

class HelpPage : public QDialog
{
    Q_OBJECT

public:
    explicit HelpPage(QWidget *parent = nullptr, QString idtopico = "");
    ~HelpPage();

private:
    Ui::HelpPage *ui;
    QString caminhoArquivoHtml;
    QString IDTOPICO;
};

#endif // HELPPAGE_H
