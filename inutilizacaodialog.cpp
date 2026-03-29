#include "inutilizacaodialog.h"
#include "ui_inutilizacaodialog.h"

InutilizacaoDialog::InutilizacaoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InutilizacaoDialog)
{
    ui->setupUi(this);
}

InutilizacaoDialog::~InutilizacaoDialog()
{
    delete ui;
}

void InutilizacaoDialog::on_Btn_Enviar_clicked()
{

}

