#include "alterarproduto.h"
#include "ui_alterarproduto.h"
#include "mainwindow.h"
#include "QSqlQuery"
#include <QMessageBox>
#include <QDoubleValidator>

AlterarProduto::AlterarProduto(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlterarProduto)
{
    ui->setupUi(this);

    // validadores para os campos
    QDoubleValidator *DoubleValidador = new QDoubleValidator(0.0, 9999.99, 2);;
    ui->Ledit_AltPreco->setValidator(DoubleValidador);
    ui->Ledit_AltQuant->setValidator(DoubleValidador);
    ui->Btn_GerarCod->setIcon(QIcon(":/QEstoqueLOja/restart.svg"));

    //
    ui->Ledit_AltDesc->setMaxLength(120);

}

AlterarProduto::~AlterarProduto()
{
    delete ui;
}

 void AlterarProduto::TrazerInfo(QString desc, QString quant, QString preco, QString barras, bool nf){
    descAlt = desc;
    quantAlt = quant;
    precoAlt = preco;
    barrasAlt = barras;
    nfAlt = nf;
    ui->Ledit_AltDesc->setText(desc);
    ui->Ledit_AltQuant->setText(quant);
    ui->Ledit_AltPreco->setText(preco);
    ui->Ledit_AltBarras->setText(barras);
    ui->Check_AltNf->setChecked(nf);
}

void AlterarProduto::on_Btn_AltAceitar_accepted()
{
    QString desc = ui->Ledit_AltDesc->text();
    QString quant = ui->Ledit_AltQuant->text();
    QString preco = ui->Ledit_AltPreco->text();
    QString barras = ui->Ledit_AltBarras->text();
    bool nf = ui->Check_AltNf->isChecked();

    // Converta o texto para um número
    bool conversionOk, conversionOkQuant;
    double price = portugues.toDouble(preco, &conversionOk);
    portugues.toFloat(quant, &conversionOkQuant);

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
                                     "Preço: " + precoAlt + "\n"
                                     "Código de Barras: " + barrasAlt + "\n"
                                      "NF: " + nfAltString  + "\n\n"
                                        "Para: \n\n"
                                        "Descrição: " + desc + "\n"
                                 "Quantidade: " + quant + "\n"
                                  "Preço: " + preco + "\n"
                                  "Código de Barras: " + barras + "\n"
                                   "NF: " + nfString + "\n\n",
                    QMessageBox::Yes | QMessageBox::No
                    );
                // Verifica a resposta do usuário
                if (resposta == QMessageBox::Yes) {
                    // alterar banco de dados
                    if(!janelaPrincipal->db.open()){
                        qDebug() << "erro ao abrir banco de dados. botao alterar->aceitar.";
                    }
                    QSqlQuery query;
                    query.prepare("UPDATE produtos SET quantidade = :valor2, descricao = :valor3, preco = :valor4, codigo_barras = :valor5, nf = :valor6 WHERE id = :valor1");
                    query.bindValue(":valor1", idAlt);
                    query.bindValue(":valor2", QString::number(portugues.toFloat(quant)));
                    query.bindValue(":valor3", MainWindow::normalizeText(desc));
                    query.bindValue(":valor4", QString::number(portugues.toFloat(preco)));
                    query.bindValue(":valor5", barras);
                    query.bindValue(":valor6", nf);
                    if (query.exec()) {
                        qDebug() << "Alteracao bem-sucedida!";
                    } else {
                        qDebug() << "Erro na alteracao: ";
                    }
                    // mostrar na tableview
                    janelaPrincipal->atualizarTableview();
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

