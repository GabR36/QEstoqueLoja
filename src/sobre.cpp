#include "sobre.h"
#include "ui_sobre.h"
#include <QFont>

Sobre::Sobre(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Sobre)
{
    ui->setupUi(this);

    QFont title_font;
    title_font.setBold(true);
    title_font.setPointSize(title_font.pointSize() + 4);
    ui->Lbl_titulo->setFont(title_font);

    ui->Lbl_version->setText(QCoreApplication::applicationVersion());
}

Sobre::~Sobre()
{
    delete ui;
}

void Sobre::on_pushButton_clicked()
{
    close();
}

