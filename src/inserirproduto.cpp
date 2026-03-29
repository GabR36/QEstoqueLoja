#include "inserirproduto.h"
#include "ui_inserirproduto.h"
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSqlQuery>
#include <QDoubleValidator>
#include "mainwindow.h"
#include "util/NfUtilidades.h"
#include "configuracao.h"
#include <QFocusEvent>
#include <QAbstractItemView>
#include "../util/codigobarrasutil.h"

InserirProduto::InserirProduto(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InserirProduto)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);

    ui->Lbl_CEST->setVisible(false);
    ui->Ledit_CEST->setVisible(false);

    ui->Ledit_CBarras->setFocus();
    ui->Ledit_Desc->setMaxLength(120);
    //add todas as unidades comerciais no combo box do header NFutilidades
    for (int i = 0; i < unidadesComerciaisCount; ++i) {
        ui->CBox_UCom->addItem(unidadesComerciais[i]);
    }
    carregarConfiguracoes();

    ui->Ledit_PercentualLucro->setText(QString::number(configDTO.porcentLucroFinanceiro));
    ui->Ledit_NCM->setText(configDTO.ncmPadraoProduto);
    ui->Ledit_CSOSN->setText(configDTO.csosnPadraoProduto);
    ui->Ledit_PIS->setText(configDTO.pisPadraoProduto);
    ui->Ledit_CEST->setText(configDTO.cestPadraoProduto);

    QIntValidator *intValidador = new QIntValidator(1,999);
    QDoubleValidator *DoubleValidador = new QDoubleValidator(0.0, 9999.99, 2);
    ui->Ledit_PrecoFinal->setValidator(DoubleValidador);
    ui->Ledit_Quant->setValidator(DoubleValidador);
    ui->Ledit_PrecoFornecedor->setValidator(DoubleValidador);
    ui->Ledit_Aliquota->setValidator(DoubleValidador);
    ui->Ledit_NCM->installEventFilter(this);

    QRegularExpression ncmRegex("^\\d{0,8}$");  // até 8 dígitos
    QRegularExpression cestRegex("^\\d{0,7}$"); // até 7 dígitos

    ui->Ledit_NCM->setValidator(new QRegularExpressionValidator(ncmRegex, this));
    ui->Ledit_CEST->setValidator(new QRegularExpressionValidator(cestRegex, this));
    ui->Ledit_CSOSN->setValidator(intValidador);
    ui->Ledit_PIS->setValidator(intValidador);
    util = new IbptUtil(this);

    //desativa campo CEST
    ui->Ledit_CEST->setEnabled(false);

    on_Ledit_NCM_editingFinished();

}

InserirProduto::~InserirProduto()
{
    delete ui;
    delete util;
}

void InserirProduto::carregarConfiguracoes(){
    Config_service *confServ = new Config_service(this);
    configDTO = confServ->carregarTudo();
}

void InserirProduto::preencherCamposProduto(const ProdutoDTO &prod)
{
    ui->Ledit_Quant->setText(portugues.toString(prod.quantidade));
    ui->Ledit_Desc->setText(prod.descricao);
    ui->Ledit_PrecoFinal->setText(prod.preco > 0 ? portugues.toString(prod.preco, 'f', 2) : "");
    ui->Ledit_CBarras->setText(prod.codigoBarras);
    ui->Check_Nf->setChecked(prod.nf);
    ui->Ledit_PrecoFornecedor->setText(prod.precoFornecedor > 0 ? portugues.toString(prod.precoFornecedor, 'f', 2) : "");
    ui->Ledit_PercentualLucro->setText(portugues.toString(prod.percentLucro));
    ui->Ledit_NCM->setText(prod.ncm);
    ui->Ledit_Aliquota->setText(prod.aliquotaIcms > 0 ? portugues.toString(prod.aliquotaIcms) : "");
    ui->Ledit_CSOSN->setText(prod.csosn);
    ui->Ledit_PIS->setText(prod.pis);
    int index = ui->CBox_UCom->findText(prod.uCom, Qt::MatchFixedString | Qt::MatchCaseSensitive);
    ui->CBox_UCom->setCurrentIndex(index >= 0 ? index : 0);
    ui->CBox_UCom->setCurrentText(prod.uCom);

    on_Ledit_NCM_editingFinished();
}

void InserirProduto::on_Btn_GerarCBarras_clicked()
{
    QString codigo = CodigoBarrasUtil::gerarNumeroCodigoBarrasNaoFiscal();
    ui->Ledit_CBarras->setText(codigo);
}

void InserirProduto::on_Ledit_CBarras_returnPressed()
{
    QString codigo = ui->Ledit_CBarras->text().trimmed();

    if (codigo.isEmpty()) {
        ui->Ledit_Desc->setFocus();
        return;
    }


    if (service.codigoBarrasExiste(codigo)) {
        QMessageBox::warning(this, "Erro",
                             "Esse código de barras já foi registrado.\n" + codigo);

        // avisa a outra janela
        emit codigoBarrasExistenteSignal(codigo);
    }

    ui->Ledit_Desc->setFocus();
}


void InserirProduto::on_Btn_Enviar_clicked()
{
    ProdutoDTO p;

    p.quantidade = portugues.toDouble(ui->Ledit_Quant->text());
    p.descricao = Produto_Service::normalizeText(ui->Ledit_Desc->text());
    p.preco = portugues.toDouble(ui->Ledit_PrecoFinal->text());
    p.codigoBarras = ui->Ledit_CBarras->text();
    p.nf = ui->Check_Nf->isChecked();
    p.uCom = ui->CBox_UCom->currentText();
    p.precoFornecedor = portugues.toDouble(ui->Ledit_PrecoFornecedor->text());
    p.percentLucro = portugues.toDouble(ui->Ledit_PercentualLucro->text());
    p.ncm = ui->Ledit_NCM->text();
    p.cest = ui->Ledit_CEST->text();
    p.aliquotaIcms = portugues.toDouble(ui->Ledit_Aliquota->text());
    p.csosn = ui->Ledit_CSOSN->text();
    p.pis = ui->Ledit_PIS->text();

    auto r = service.inserir(p);

    if (!r.ok) {

        switch (r.erro) {

        case ProdutoErro::NcmAviso: {
            auto reply = QMessageBox::question(
                this,
                "Aviso",
                "O campo NCM está errado.\n"
                "Produtos sem NCM não poderão ser utilizados na emissão de nota fiscal.\n"
                "Deseja salvar o produto mesmo assim?",
                QMessageBox::Yes | QMessageBox::No
                );

            if (reply == QMessageBox::No)
                return;

            // versão futura:
            // auto r2 = service.inserirForcado(p);
            // if (!r2.ok) {
            //     QMessageBox::warning(this, "Erro", r2.mensagem);
            //     return;
            // }

            break;
        }

        default:
            QMessageBox::warning(this, "Erro", r.msg);
            return;
        }
    }

    emit produtoInserido();
    this->close();
}

void InserirProduto::on_Ledit_PrecoFornecedor_textChanged(const QString &arg1)
{
    if (atualizando) return;

    bool ok1 = false, ok2 = false;

    double precoFornecedor = portugues.toDouble(arg1, &ok1);
    double percentualLucro = portugues.toDouble(ui->Ledit_PercentualLucro->text(), &ok2);

    if (!ok1 || !ok2) return;

    atualizando = true;

    double precoFinal = service.calcularPrecoFinal(precoFornecedor, percentualLucro);

    ui->Ledit_PrecoFinal->setText(portugues.toString(precoFinal, 'f', 2));

    atualizando = false;
}


void InserirProduto::on_Ledit_PercentualLucro_textChanged(const QString &arg1)
{
    if (atualizando) return;
    atualizando = true;

    bool ok1, ok2;
    double precoFornecedor = portugues.toDouble(ui->Ledit_PrecoFornecedor->text(), &ok1);
    double percentualLucro = portugues.toDouble(arg1, &ok2);

    if (ok1 && ok2) {
        double precoFinal = service.calcularPrecoFinal(precoFornecedor, percentualLucro);
        ui->Ledit_PrecoFinal->setText(portugues.toString(precoFinal, 'f', 2));
    }

    atualizando = false;
}


void InserirProduto::on_Ledit_PrecoFinal_textChanged(const QString &arg1)
{
    if (atualizando) return;
    atualizando = true;

    bool ok1, ok2;
    double precoFornecedor = portugues.toDouble(ui->Ledit_PrecoFornecedor->text(), &ok1);
    double precoFinal = portugues.toDouble(arg1, &ok2);

    if (ok1 && ok2 && precoFornecedor != 0.0) {
        double percentualLucro = service.calcularPercentualLucro(precoFornecedor, precoFinal);
        ui->Ledit_PercentualLucro->setText(portugues.toString(percentualLucro, 'f', 2));
    }

    atualizando = false;
}


void InserirProduto::on_Ledit_NCM_editingFinished()
{
    QString ncmText = ui->Ledit_NCM->text();
    if(!util->eh_Valido_NCM(ncmText)){
        QMessageBox::warning(this, "Atenção", "NCM inválido para NF");
        ui->Lbl_NcmDesc->setText("Não Encontrado");


    }else{
        QString textoFormatado = portugues.toString(util->get_Aliquota_From_Csv(ncmText));
        ui->Ledit_Aliquota->setText(textoFormatado);
        ui->Lbl_NcmDesc->setText(util->get_Descricao_NCM(ncmText));
    }

}

void InserirProduto::on_Ledit_Desc_editingFinished()
{
    //atualizarCompleterNCM();
}



