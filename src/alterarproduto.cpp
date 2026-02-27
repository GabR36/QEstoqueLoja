#include "alterarproduto.h"
#include "ui_alterarproduto.h"
#include "mainwindow.h"
#include "QSqlQuery"
#include <QMessageBox>
#include <QDoubleValidator>
#include "util/NfUtilidades.h"
#include "util/codigobarrasutil.h"

AlterarProduto::AlterarProduto(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlterarProduto)
{
    ui->setupUi(this);
    util->setParent(this);
    db = QSqlDatabase::database();

    ui->Lbl_AltCEST->setVisible(false);
    ui->Ledit_AltCEST->setVisible(false);

    ui->tabWidget->setCurrentIndex(0);
    // validadores para os campos
    QDoubleValidator *DoubleValidador = new QDoubleValidator(0.0, 9999.99, 2);
    QIntValidator *intValidador = new QIntValidator(1,999);

    ui->Ledit_AltPreco->setValidator(DoubleValidador);
    ui->Ledit_AltQuant->setValidator(DoubleValidador);
    ui->Ledit_AltPrecoFornecedor->setValidator(DoubleValidador);
    ui->Ledit_AltPercentualLucro->setValidator(DoubleValidador);
    QRegularExpression ncmRegex("^\\d{0,8}$");  // até 8 dígitos
    QRegularExpression cestRegex("^\\d{0,7}$"); // até 7 dígitos

    ui->Ledit_AltNCM->setValidator(new QRegularExpressionValidator(ncmRegex, this));
    ui->Ledit_AltCEST->setValidator(new QRegularExpressionValidator(cestRegex, this));
    ui->Ledit_AltCSOSN->setValidator(intValidador);
    ui->Ledit_AltPIS->setValidator(intValidador);

    ui->Btn_GerarCod->setIcon(QIcon(":/QEstoqueLOja/restart.svg"));

    ui->Ledit_AltDesc->setMaxLength(120);
    //add todas as unidades comerciais no combo box do header NFutilidades
    for (int i = 0; i < unidadesComerciaisCount; ++i) {
        ui->CBox_AltUCom->addItem(unidadesComerciais[i]);
    }
    //desativa campo cest
    ui->Ledit_AltCEST->setEnabled(false);
    produtoService = new Produto_Service();


}

AlterarProduto::~AlterarProduto()
{
    delete ui;
    delete util;
}

 void AlterarProduto::TrazerInfo(QString desc, QString quant, QString preco, QString barras, bool nf,
                                QString ucom, QString precoforn, QString porcentlucro, QString ncm,
                                QString cest, QString aliquotaimp, QString csosn, QString pis){


    descAlt = desc;
    quantAlt = quant;
    precoAlt = preco;
    barrasAlt = barras;
    nfAlt = nf;
    ucomAlt = ucom;
    precoFornAlt = precoforn;
    porcentLucroAlt = porcentlucro;
    ncmAlt = ncm;
    cestAlt = cest;
    aliquotaImpAlt = aliquotaimp;
    csosnAlt = csosn;
    pisAlt = pis;

    ui->Ledit_AltDesc->setText(desc);
    ui->Ledit_AltQuant->setText(quant);
    ui->Ledit_AltPreco->setText(preco);
    ui->Ledit_AltBarras->setText(barras);
    ui->Check_AltNf->setChecked(nf);
    ui->Ledit_AltPrecoFornecedor->setText(precoforn);
    ui->Ledit_AltPercentualLucro->setText(porcentlucro);
    ui->CBox_AltUCom->setCurrentText(ucom);
    ui->Ledit_AltNCM->setText(ncm);
    ui->Ledit_AltCEST->setText(cest);
    ui->Ledit_AltAliquota->setText(aliquotaimp);
    ui->Ledit_AltCSOSN->setText(csosn);
    ui->Ledit_AltPIS->setText(pis);

    ui->Lbl_AltNCMDesc->setText(util->get_Descricao_NCM(ncm));
}

 void AlterarProduto::on_Btn_AltAceitar_accepted()
 {
     ProdutoDTO p;

     // leitura da UI
     p.descricao = ui->Ledit_AltDesc->text();
     p.quantidade = portugues.toFloat(ui->Ledit_AltQuant->text());
     p.preco = portugues.toDouble(ui->Ledit_AltPreco->text());
     p.codigoBarras = ui->Ledit_AltBarras->text();
     p.nf = ui->Check_AltNf->isChecked();
     p.precoFornecedor = portugues.toDouble(ui->Ledit_AltPrecoFornecedor->text());
     p.percentLucro = portugues.toDouble(ui->Ledit_AltPercentualLucro->text());
     p.uCom = ui->CBox_AltUCom->currentText();
     p.ncm = ui->Ledit_AltNCM->text();
     p.cest = ui->Ledit_AltCEST->text();
     p.aliquotaIcms = portugues.toDouble(ui->Ledit_AltAliquota->text());
     p.csosn = ui->Ledit_AltCSOSN->text();
     p.pis = ui->Ledit_AltPIS->text();

     auto res = produtoService->alterarVerificarCodigoBarras(p, barrasAlt, idAlt);

     if (!res.ok) {
         QMessageBox::warning(this, "Erro", res.msg);
         return;
     }

     emit produtoAlterado();
     accept();
 }



void AlterarProduto::on_Btn_GerarCod_clicked()
{
    QString codigo = CodigoBarrasUtil::gerarNumeroCodigoBarrasNaoFiscal();
    ui->Ledit_AltBarras->setText(codigo);
}

void AlterarProduto::on_Ledit_AltNCM_editingFinished()
{
    QString texto = ui->Ledit_AltNCM->text();
    QString textoFormatado = portugues.toString(util->get_Aliquota_From_Csv(texto));
    ui->Ledit_AltAliquota->setText(textoFormatado);
    ui->Lbl_AltNCMDesc->setText(util->get_Descricao_NCM(texto));
}


void AlterarProduto::on_Ledit_AltPrecoFornecedor_textChanged(const QString &arg1)
{
    if (atualizando) return;

    double precoFornecedor = portugues.toDouble(arg1);
    float percentualLucro = portugues.toFloat(ui->Ledit_AltPercentualLucro->text());

    atualizando = true;
    double precoFinal = produtoService->calcularPrecoFinal(precoFornecedor, percentualLucro);
    ui->Ledit_AltPreco->setText(portugues.toString(precoFinal, 'f', 2));
    atualizando = false;
}

void AlterarProduto::on_Ledit_AltPercentualLucro_textChanged(const QString &arg1)
{
    if (atualizando) return;
    atualizando = true;

    bool ok1, ok2;
    double precoFornecedor = portugues.toDouble(ui->Ledit_AltPrecoFornecedor->text(), &ok1);
    double percentualLucro = portugues.toDouble(arg1, &ok2);

    if (ok1 && ok2) {
        double precoFinal = produtoService->calcularPrecoFinal(precoFornecedor, percentualLucro);
        ui->Ledit_AltPreco->setText(portugues.toString(precoFinal, 'f', 2));
    }

    atualizando = false;
}



void AlterarProduto::on_Ledit_AltPreco_textChanged(const QString &arg1)
{
    if (atualizando) return;
    atualizando = true;

    bool ok1, ok2;
    double precoFornecedor = portugues.toDouble(ui->Ledit_AltPrecoFornecedor->text(), &ok1);
    double precoFinal = portugues.toDouble(arg1, &ok2);

    if (ok1 && ok2 && precoFornecedor != 0.0) {
        double percentualLucro = produtoService->calcularPercentualLucro(precoFornecedor,
                                                                         precoFinal);
        ui->Ledit_AltPercentualLucro->setText(portugues.toString(percentualLucro, 'f', 2));
    }

    atualizando = false;
}

