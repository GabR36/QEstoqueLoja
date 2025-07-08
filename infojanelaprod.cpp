#include "infojanelaprod.h"
#include "ui_infojanelaprod.h"
#include <QDebug>
#include <QSqlQuery>

InfoJanelaProd::InfoJanelaProd(QWidget *parent, int id)
    : QDialog(parent)
    , ui(new Ui::InfoJanelaProd)
{
    ui->setupUi(this);
    ui->Lbl_ID->setText(QString::number(id));

    if(!db.open()){
        qDebug() << "nao abriu bd janela info prod";
    }
    QSqlQuery query;
    query.prepare("SELECT quantidade, descricao, preco, codigo_barras, nf, un_comercial,"
                  "preco_fornecedor, porcent_lucro, ncm, cest, aliquota_imposto, csosn, pis, "
                  "local FROM produtos WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Erro ao executar query:" << query.lastError();
    }
    while (query.next()) {
        quant = query.value(0).toString();
        desc = query.value(1).toString();
        precoFinal = portugues.toString(query.value(2).toFloat());
        codigoBarras = query.value(3).toString();
        nf = query.value(4).toBool();
        ucom = query.value(5).toString();
        precoForn = portugues.toString(query.value(6).toFloat());
        porcentLucro = portugues.toString(query.value(7).toFloat());
        ncm = query.value(8).toString();
        cest = query.value(9).toString();
        aliquotaIcms = portugues.toString(query.value(10).toFloat());
        csosn = query.value(11).toString();
        pis = query.value(12).toString();
        local = query.value(13).toString();
    }
    db.close();
    ui->Lbl_Quant->setText(quant);
    ui->Lbl_Desc->setText(desc);
    ui->Lbl_CBarras->setText(codigoBarras);
    ui->Lbl_UCom->setText(ucom);
    ui->Lbl_PrecoForn->setText(precoForn);
    ui->Lbl_PrecoFinal->setText(precoFinal);
    ui->Lbl_PorcentLucro->setText(porcentLucro);
    ui->Lbl_NCM->setText(ncm);
    if(nf){
        ui->Lbl_Nf->setText("Sim");
    }{
        ui->Lbl_Nf->setText("Não");
    }
    ui->Lbl_AliquotaICMS->setText(aliquotaIcms);
    ui->Lbl_Csosn->setText(csosn);
    ui->Lbl_Pis->setText(pis);
    ui->Lbl_Local->setText(local);

}

InfoJanelaProd::~InfoJanelaProd()
{
    delete ui;
}
