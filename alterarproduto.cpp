#include "alterarproduto.h"
#include "ui_alterarproduto.h"
#include "mainwindow.h"
#include "QSqlQuery"
#include <QMessageBox>
#include <QDoubleValidator>
#include "util/NfUtilidades.h"

AlterarProduto::AlterarProduto(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlterarProduto)
{
    ui->setupUi(this);
    util->setParent(this);

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



    //
    ui->Ledit_AltDesc->setMaxLength(120);
    //add todas as unidades comerciais no combo box do header NFutilidades
    for (int i = 0; i < unidadesComerciaisCount; ++i) {
        ui->CBox_AltUCom->addItem(unidadesComerciais[i]);
    }

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
    QString desc = ui->Ledit_AltDesc->text();
    QString quant = ui->Ledit_AltQuant->text();
    QString preco = ui->Ledit_AltPreco->text();
    QString barras = ui->Ledit_AltBarras->text();
    bool nf = ui->Check_AltNf->isChecked();
    QString precoForn = ui->Ledit_AltPrecoFornecedor->text();
    QString porcentLucro = ui->Ledit_AltPercentualLucro->text();
    QString uCom = ui->CBox_AltUCom->currentText();
    QString ncm = ui->Ledit_AltNCM->text();
    QString cest = ui->Ledit_AltCEST->text();
    QString aliquotaImp = ui->Ledit_AltAliquota->text();
    QString csosn = ui->Ledit_AltCSOSN->text();
    QString pis = ui->Ledit_AltPIS->text();


    // Converta o texto para um número
    bool conversionOk, conversionOkQuant;
    double price = portugues.toDouble(preco, &conversionOk);
    portugues.toFloat(quant, &conversionOkQuant);
    if (porcentLucro.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erro", "O campo 'Percentual de Lucro' não pode estar vazio.");
        ui->Ledit_AltPercentualLucro->setFocus();
        return;
    }
    if(precoForn.trimmed().isEmpty()){
        precoForn = "";
    }else{
        precoForn = QString::number(portugues.toDouble(precoForn));
    }
    if ((ncm.trimmed().isEmpty() && nf) || (ncm.length() != 8 && nf)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Aviso",
            "O campo NCM está errado.\n"
            "Produtos sem NCM não poderão ser utilizados na emissão de nota fiscal.\n"
            "Deseja salvar o produto mesmo assim?",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::No) {
            ui->Ledit_AltNCM->setFocus();
            return; // cancela o envio
        }
    }
    porcentLucro = QString::number(portugues.toFloat(porcentLucro));
    aliquotaImp = QString::number(portugues.toFloat(aliquotaImp));
    // Verifique se a conversão foi bem-sucedida e se o preço é maior que zero
    if (conversionOk && price >= 0)
    {
        if (conversionOkQuant){
            // verificar se o codigo de barras ja existe
            if(!janelaPrincipal->db.open()){
                qDebug() << "erro ao abrir banco de dados. botao alterar.";
            }
            QSqlQuery query;

            query.prepare("SELECT COUNT(*) FROM produtos WHERE codigo_barras = :codigoBarras");
            query.bindValue(":codigoBarras", barras);
            if (!query.exec()) {
                qDebug() << "Erro na consulta: contagem codigo barras";
            }
            query.next();
            bool barrasExiste = query.value(0).toInt() > 0 && barras != "" && barras != barrasAlt;
            qDebug() << barras;
            if (!barrasExiste){
                QString nfAltString = nfAlt ? "1" : "0";
                QString nfString = nf ? "1" : "0";
                // Cria uma mensagem de confirmação
                QMessageBox::StandardButton resposta;
                resposta = QMessageBox::question(
                    nullptr,
                    "Confirmação",
                    "Tem certeza que deseja alterar o produto:\n\n"
                    "id: " + idAlt + "\n"
                                  "Descrição: " + descAlt + "\n"
                                    "Quantidade: " + quantAlt + "\n"
                                     "UCom: " + ucomAlt + "\n"
                                     "Preço Fornecedor: " + precoFornAlt + "\n"
                                     "Porcentagem Lucro: " + porcentLucroAlt + "\n"
                                     "Preço: " + precoAlt + "\n"
                                     "Código de Barras: " + barrasAlt + "\n"
                                      "NF: " + nfAltString  + "\n\n"
                                        "NCM: " + ncmAlt + "\n"
                                   "CEST: " + cestAlt + "\n"
                                    "Aliquota Imp: " + aliquotaImpAlt + "\n"
                                    "CSOSN: " + csosnAlt + "\n"
                                     "PIS: " +  pisAlt + "\n"

                                        "Para: \n\n"
                                        "Descrição: " + desc + "\n"
                                 "Quantidade: " + quant + "\n"
                                  "UCom: " + uCom + "\n"
                                 "Preço Fornecedor: " + precoForn + "\n"
                                "Porcentagem Lucro: " + porcentLucro + "\n"
                                  "Preço: " + preco + "\n"
                                  "Código de Barras: " + barras + "\n"
                                   "NF: " + nfString + "\n\n"
                                     "NCM: " + ncm + "\n"
                                   "CEST: " + cest + "\n"
                                    "Aliquota Imp: " + aliquotaImp + "\n"
                                    "CSOSN: " + csosn + "\n"
                                    "PIS: " +  pis + "\n",

                    QMessageBox::Yes | QMessageBox::No
                    );
                // Verifica a resposta do usuário
                if (resposta == QMessageBox::Yes) {
                    // alterar banco de dados
                    if(!janelaPrincipal->db.open()){
                        qDebug() << "erro ao abrir banco de dados. botao alterar->aceitar.";
                    }
                    QSqlQuery query;
                    query.prepare("UPDATE produtos SET quantidade = :valor2, descricao = :valor3, preco = :valor4, "
                                  "codigo_barras = :valor5, nf = :valor6, un_comercial = :ucom,"
                                  "preco_fornecedor = :precoforn, porcent_lucro = :porcentlucro,"
                                  "ncm = :ncm, cest = :cest, aliquota_imposto = :aliquotaimp,"
                                  "csosn = :csosn, pis = :pis  WHERE id = :valor1");
                    query.bindValue(":valor1", idAlt);
                    query.bindValue(":valor2", QString::number(portugues.toFloat(quant)));
                    query.bindValue(":valor3", MainWindow::normalizeText(desc));
                    query.bindValue(":valor4", QString::number(portugues.toFloat(preco)));
                    query.bindValue(":valor5", barras);
                    query.bindValue(":valor6", nf);
                    query.bindValue(":ucom", uCom);
                    query.bindValue(":precoforn", precoForn);
                    query.bindValue(":porcentlucro", porcentLucro);
                    query.bindValue(":ncm", ncm);
                    query.bindValue(":cest", cest);
                    query.bindValue(":aliquotaimp", aliquotaImp);
                    query.bindValue(":csosn", csosn);
                    query.bindValue(":pis", pis);
                    if (query.exec()) {
                        qDebug() << "Alteracao bem-sucedida!";
                    } else {
                        qDebug() << "Erro na alteracao: ";
                    }
                    // mostrar na tableview
                    emit produtoAlterado();
                   // janelaPrincipal->atualizarTableview();
                    QSqlDatabase::database().close();
                }
                else {
                    // O usuário escolheu não alterar o produto
                    qDebug() << "A alteraçao do produto foi cancelada.";
                }
            }
            else {
                QMessageBox::warning(this, "Erro", "Esse código de barras já foi registrado.");
            }
        }
        else {
            // a quantidade é invalida
            QMessageBox::warning(this, "Erro", "Por favor, insira uma quantiade válida.");
            ui->Ledit_AltQuant->setFocus();
        }

    }
    else
    {
        // Exiba uma mensagem de erro se o preço não for válido
        QMessageBox::warning(this, "Erro", "Por favor, insira um preço válido.");
    }
}


void AlterarProduto::on_Btn_GerarCod_clicked()
{
    ui->Ledit_AltBarras->setText(janelaPrincipal->gerarNumero());
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
    double precoFinal = precoFornecedor * (1.0 + percentualLucro / 100.0);
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
        double precoFinal = precoFornecedor * (1.0 + percentualLucro / 100.0);
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
        double percentualLucro = ((precoFinal - precoFornecedor) / precoFornecedor) * 100.0;
        ui->Ledit_AltPercentualLucro->setText(portugues.toString(percentualLucro, 'f', 2));
    }

    atualizando = false;
}

