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

InserirProduto::InserirProduto(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InserirProduto)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    financeiroValues = Configuracao::get_All_Financeiro_Values();
    produtoValues = Configuracao::get_All_Produto_Values();
    ui->Ledit_CBarras->setFocus();
    ui->Ledit_Desc->setMaxLength(120);
    //add todas as unidades comerciais no combo box do header NFutilidades
    for (int i = 0; i < unidadesComerciaisCount; ++i) {
        ui->CBox_UCom->addItem(unidadesComerciais[i]);
    }

    ui->Ledit_PercentualLucro->setText(financeiroValues.value("porcent_lucro"));
    ui->Ledit_NCM->setText(produtoValues.value("ncm_padrao"));
    ui->Ledit_CSOSN->setText(produtoValues.value("csosn_padrao"));
    ui->Ledit_PIS->setText(produtoValues.value("pis_padrao"));
    ui->Ledit_CEST->setText(produtoValues.value("cest_padrao"));

    QIntValidator *intValidador = new QIntValidator(1,999);
    QDoubleValidator *DoubleValidador = new QDoubleValidator(0.0, 9999.99, 2);
    ui->Ledit_PrecoFinal->setValidator(DoubleValidador);
    ui->Ledit_Quant->setValidator(DoubleValidador);
    ui->Ledit_PrecoFornecedor->setValidator(DoubleValidador);
    ui->Ledit_NCM->installEventFilter(this);

    QRegularExpression ncmRegex("^\\d{0,8}$");  // até 8 dígitos
    QRegularExpression cestRegex("^\\d{0,7}$"); // até 7 dígitos

    ui->Ledit_NCM->setValidator(new QRegularExpressionValidator(ncmRegex, this));
    ui->Ledit_CEST->setValidator(new QRegularExpressionValidator(cestRegex, this));
    ui->Ledit_CSOSN->setValidator(intValidador);
    ui->Ledit_PIS->setValidator(intValidador);
    util = new IbptUtil(this);

}

InserirProduto::~InserirProduto()
{
    delete ui;
    delete util;
}
QString InserirProduto::gerarNumero()
{
    QString number;
    do {
        number = QString("3562%1").arg(QRandomGenerator::global()->bounded(100000), 5, 10, QChar('0'));
    } while (generatedNumbers.contains(number));

    generatedNumbers.insert(number);
    // saveGeneratedNumber(number);

    return number;
}
void InserirProduto::on_Btn_GerarCBarras_clicked()
{
    ui->Ledit_CBarras->setText(gerarNumero());
}

void InserirProduto::on_Ledit_CBarras_returnPressed()
{
    // verificar se o codigo de barras existe
    verificarCodigoBarras();
    ui->Ledit_Desc->setFocus();
}

bool InserirProduto::verificarCodigoBarras(){
    QString barrasProduto = ui->Ledit_CBarras->text();
    // verificar se o codigo de barras ja existe
    if(!db.open()){
        qDebug() << "erro ao abrir banco de dados. botao enviar.";
    }
    QSqlQuery query;

    query.prepare("SELECT COUNT(*) FROM produtos WHERE codigo_barras = :codigoBarras");
    query.bindValue(":codigoBarras", barrasProduto);
    if (!query.exec()) {
        qDebug() << "Erro na consulta: contagem codigo barras";
    }
    query.next();
    bool barrasExiste = query.value(0).toInt() > 0 && barrasProduto != "";
    qDebug() << barrasProduto;

    if (barrasExiste){
        // codigo de barras existe, mostrar mensagem e
        // mostrar registro na tabela
        QMessageBox::warning(this, "Erro", "Esse código de barras já foi registrado.\n" + barrasProduto );
        if(!db.open()){
            qDebug() << "erro ao abrir banco de dados. codigo de barras existente";
        }
        QString query = "SELECT * FROM produtos WHERE codigo_barras = '" + barrasProduto + "'";
        emit codigoBarrasExistenteSignal(query);
        db.close();
        return true;
    }
    else{
        return false;
    }
}


void InserirProduto::on_Btn_Enviar_clicked()
{
    QString quantidadeProduto, descProduto, precoProduto, barrasProduto, uCom, precoFornecedor,
        percentLucro, ncm, cest, aliquotaIcms, csosn, pis;
    bool nfProduto;
    quantidadeProduto = ui->Ledit_Quant->text();
    descProduto = MainWindow::normalizeText(ui->Ledit_Desc->text());
    precoProduto = ui->Ledit_PrecoFinal->text();
    barrasProduto = ui->Ledit_CBarras->text();
    nfProduto = ui->Check_Nf->isChecked();
    uCom = ui->CBox_UCom->currentText();
    precoFornecedor = ui->Ledit_PrecoFornecedor->text();
    percentLucro = ui->Ledit_PercentualLucro->text();
    ncm = ui->Ledit_NCM->text();
    cest = ui->Ledit_CEST->text();
    aliquotaIcms = ui->Ledit_Aliquota->text();
    csosn = ui->Ledit_CSOSN->text();
    pis = ui->Ledit_PIS->text();

    if (percentLucro.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erro", "O campo 'Percentual de Lucro' não pode estar vazio.");
        ui->Ledit_PercentualLucro->setFocus();
        return;
    }
    if (ncm.trimmed().isEmpty() && nfProduto) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Aviso",
            "O campo NCM está vazio.\n"
            "Produtos sem NCM não poderão ser utilizados na emissão de nota fiscal.\n"
            "Tente utilizar uma das sugestões de NCM clicando no campo.\n\n"
            "Deseja salvar o produto mesmo assim?",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::No) {
            ui->Ledit_NCM->setFocus();
            return; // cancela o envio
        }
    }
    // Converta o texto para um número
    bool conversionOk;
    bool conversionOkQuant;
    double price = portugues.toDouble(precoProduto, &conversionOk);
    qDebug() << price;
    // quantidade precisa ser transformada com ponto para ser armazenada no db
    quantidadeProduto = QString::number(portugues.toFloat(quantidadeProduto, &conversionOkQuant));
    precoFornecedor = QString::number(portugues.toDouble(precoFornecedor));
    percentLucro = QString::number(portugues.toFloat(percentLucro));
    aliquotaIcms = QString::number(portugues.toFloat(aliquotaIcms));
    // Verifique se a conversão foi bem-sucedida e se o preço é maior que zero
    //qDebug() << quantidadeProduto << " " << precoFornecedor;
    if (conversionOk && price >= 0)
    {
        if (conversionOkQuant){
            // guardar no banco de dados o valor notado em local da linguagem
            precoProduto = QString::number(price, 'f', 2);
            qDebug() << precoProduto;
            // verificar se o codigo de barras ja existe
            if (!verificarCodigoBarras()){
                // adicionar ao banco de dados
                if(!db.open()){
                    qDebug() << "erro ao abrir banco de dados. botao enviar.";
                }
                QSqlQuery query;

                query.prepare("INSERT INTO produtos (quantidade, descricao, preco, codigo_barras, nf, "
                              "un_comercial, preco_fornecedor, porcent_lucro, ncm, cest, aliquota_imposto,"
                              "csosn, pis)"
                              " VALUES (:valor1, :valor2, :valor3, :valor4, :valor5, :ucom, :preco_forn, "
                              ":porcent_lucro, :ncm, :cest, :aliquota_imp, :csosn, :pis)");
                query.bindValue(":valor1", quantidadeProduto);
                query.bindValue(":valor2", descProduto);
                query.bindValue(":valor3", precoProduto);
                query.bindValue(":valor4", barrasProduto);
                query.bindValue(":valor5", nfProduto);
                query.bindValue(":ucom", uCom);
                query.bindValue(":preco_forn", precoFornecedor);
                query.bindValue(":porcent_lucro", percentLucro);
                query.bindValue(":ncm", ncm);
                query.bindValue(":cest", cest);
                query.bindValue(":aliquota_imp", aliquotaIcms);
                query.bindValue(":csosn", csosn);
                query.bindValue(":pis", pis);

                if (query.exec()) {
                    qDebug() << "Inserção bem-sucedida!";
                } else {
                    qDebug() << "Erro na inserção: ";
                }
                emit produtoInserido();
                db.close();
                this->close(); // fecha janela


            }
        }
        else {
            // a quantidade é invalida
            QMessageBox::warning(this, "Erro", "Por favor, insira uma quantiade válida.");
            ui->Ledit_Quant->setFocus();
        }
    }
    else
    {
        // Exiba uma mensagem de erro se o preço não for válido
        QMessageBox::warning(this, "Erro", "Por favor, insira um preço válido.");
        ui->Ledit_PrecoFinal->clear();
    }

}


void InserirProduto::on_Ledit_PrecoFornecedor_textChanged(const QString &arg1)
{
    if (atualizando) return;

    double precoFornecedor = portugues.toDouble(arg1);
    float percentualLucro = portugues.toFloat(ui->Ledit_PercentualLucro->text());

    atualizando = true;
    double precoFinal = precoFornecedor * (1.0 + percentualLucro / 100.0);
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
        double precoFinal = precoFornecedor * (1.0 + percentualLucro / 100.0);
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
        double percentualLucro = ((precoFinal - precoFornecedor) / precoFornecedor) * 100.0;
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



