#include "inutilizacaodialog.h"
#include "ui_inutilizacaodialog.h"
#include "services/fiscalemitter_service.h"
#include "qmessagebox.h"

InutilizacaoDialog::InutilizacaoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InutilizacaoDialog)
{
    ui->setupUi(this);

    ui->CBox_Amb->addItem("Homologação");
    ui->CBox_Amb->addItem("Produção");

    configDTO = configServ.carregarTudo();

    if(configDTO.tpAmbFiscal == 0){
        ui->CBox_Amb->setCurrentIndex(0);
    }else if(configDTO.tpAmbFiscal == 1){
        ui->CBox_Amb->setCurrentIndex(1);
    }
    ui->CBox_Amb->setDisabled(true);
    ui->CBox_TipoNF->addItem("NFCe");
    ui->CBox_TipoNF->addItem("NFe");

    ui->Ledit_Motivo->setText("Erro de sequência numérica no sistema");
}

InutilizacaoDialog::~InutilizacaoDialog()
{
    delete ui;
}

void InutilizacaoDialog::on_Btn_Enviar_clicked()
{
    qlonglong numIni = ui->Ledit_NumIni->text().toLongLong();
    qlonglong numFinal = ui->Ledit_NumFinal->text().toLongLong();
    QString motivo = ui->Ledit_Motivo->text();
    ModeloNF modelo = ui->CBox_TipoNF->currentIndex() == 0 ? ModeloNF::NFCe : ModeloNF::NFE;

    FiscalEmitter_service fiscalServ;
    auto result = fiscalServ.enviarInutilizacao(modelo, motivo, numIni, numFinal);

    QMessageBox::information(this, "Info", result.msg);
}

