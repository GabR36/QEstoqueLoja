#include "cartacorrecaojanela.h"
#include "ui_cartacorrecaojanela.h"
#include <QIntValidator>
#include <QMessageBox>

CartaCorrecaoJanela::CartaCorrecaoJanela(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CartaCorrecaoJanela)
{
    ui->setupUi(this);
    ui->Ledit_Nseq->setText("1");
    QIntValidator *intValidador = new QIntValidator(1,999);
    ui->Ledit_Nseq->setValidator(intValidador);

}

CartaCorrecaoJanela::~CartaCorrecaoJanela()
{
    delete ui;
}

void CartaCorrecaoJanela::on_Btn_Enviar_clicked()
{
    QString correcao = ui->Ledit_Correcao->text();
    QString chave = ui->Ledit_Chnfe->text();
    int nseq = ui->Ledit_Nseq->text().toInt();

    auto result = eventoServ.enviarCCE(chave, nseq, correcao);
    if (!result.ok){
        QMessageBox::warning(this, "Erro", result.msg);
        return;
    }else{
        QMessageBox::information(this, "OK", result.msg);
        this->close();
    }
}

