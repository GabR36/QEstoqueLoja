#include "infojanelaprod.h"
#include "ui_alterarproduto.h"
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QDialogButtonBox>

InfoJanelaProd::InfoJanelaProd(QWidget *parent, QString id)
    : AlterarProduto(parent)
{
    setWindowTitle("Informação Produto - QEstoqueLoja");
    idAlt = id;

    ProdutoDTO p = infoService.getProduto(id.toLongLong());

    TrazerInfo(p);

    // Campos readonly
    ui->Ledit_AltBarras->setReadOnly(true);
    ui->Ledit_AltDesc->setReadOnly(true);
    ui->Ledit_AltQuant->setReadOnly(true);
    ui->Ledit_AltPrecoFornecedor->setReadOnly(true);
    ui->Ledit_AltPercentualLucro->setReadOnly(true);
    ui->Ledit_AltPreco->setReadOnly(true);
    ui->Ledit_AltNCM->setReadOnly(true);
    ui->Ledit_AltAliquota->setReadOnly(true);
    ui->Ledit_AltCSOSN->setReadOnly(true);
    ui->Ledit_AltPIS->setReadOnly(true);
    ui->Ledit_AltCEST->setReadOnly(true);
    ui->CBox_AltUCom->setEnabled(false);
    ui->Check_AltNf->setEnabled(false);
    ui->Btn_GerarCod->setVisible(false);

    // Botão: só Fechar
    ui->Btn_AltAceitar->button(QDialogButtonBox::Ok)->setVisible(false);
    ui->Btn_AltAceitar->button(QDialogButtonBox::Cancel)->setText("Fechar");

    // Campo local — inserido na aba Principal acima do groupBox Preços
    ui->groupBox->move(ui->groupBox->x(), ui->groupBox->y() + 32);

    QWidget *localWidget = new QWidget(ui->tab_1);
    localWidget->setGeometry(10, 148, 311, 24);
    QHBoxLayout *localLayout = new QHBoxLayout(localWidget);
    localLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *localLabel = new QLabel("Local:", localWidget);
    QLineEdit *localEdit = new QLineEdit(localWidget);
    localEdit->setReadOnly(true);
    localEdit->setText(p.local);

    localLayout->addWidget(localLabel);
    localLayout->addWidget(localEdit);
}
