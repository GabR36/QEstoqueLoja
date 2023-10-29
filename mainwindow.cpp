#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "produto.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // começo leitura do arquivo
    QFile arquivo("../QEstoqueLoja/estoque.xml");
    if (!arquivo.exists()){
        if (arquivo.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QXmlStreamWriter xmlWriter(&arquivo);
            xmlWriter.setAutoFormatting(true);
            xmlWriter.writeStartDocument();
            xmlWriter.writeStartElement("produtos");
            arquivo.close();
        }
        qDebug() << "erro ao iniciar arquivo xml.";
    }
    if (arquivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QXmlStreamReader xmlReader(&arquivo);
        QString nome, quant, desc;
        while(!xmlReader.atEnd()){
            xmlReader.readNext();
            if (xmlReader.isStartElement()){
                if (xmlReader.name() == QStringLiteral("nome")){
                    xmlReader.readNext();
                    nome = xmlReader.text().toString();
                }
                else {
                    if (xmlReader.name() == QStringLiteral("quantidade")){
                        xmlReader.readNext();
                        quant = xmlReader.text().toString();
                    }
                    else {
                        if (xmlReader.name() == QStringLiteral("descricao")){
                            xmlReader.readNext();
                            desc = xmlReader.text().toString();
                        }
                    }
                }
            }
            if (xmlReader.isEndElement() && xmlReader.name() == QStringLiteral("produto")) {
                Produto novoProduto(nome, desc, quant.toInt());
                produtos.push_back(novoProduto);
                qDebug() << produtos[1].nome;
            }
        }
        arquivo.close();
    }
    else {
        QMessageBox::warning(this,"ERRO", "Algo deu errado ao abrir o arquivo.");
    }
    // fim da leitura do arquivo
    ui->Ledit_Nome->setFocus();
    model->setHorizontalHeaderItem(0, new QStandardItem("Nome"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Quantidade"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Descrição"));
    ui->Tview_Produtos->setModel(model);
    ui->Tview_Produtos->setColumnWidth(2, 165);

    for(int i = 0;i < produtos.size(); i++){
        QStandardItem *newNome = new QStandardItem(produtos[i].nome);
        QStandardItem *newQuantidade = new QStandardItem(QString::number(produtos[i].quantidade));
        QStandardItem *newDesc = new QStandardItem(produtos[i].descricao);
        rowCount = model->rowCount();
        model->setItem(rowCount, 0, newNome);
        model->setItem(rowCount, 1, newQuantidade);
        model->setItem(rowCount, 2, newDesc);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_Btn_Enviar_clicked()
{
    nomeProduto = ui->Ledit_Nome->text();
    quantidadeProduto = ui->Ledit_Quantidade->text();
    descProduto = ui->Ledit_Desc->text();
    Produto addProduto(nomeProduto, descProduto, quantidadeProduto.toInt());
    produtos.push_back(addProduto);
    QStandardItem *newNome = new QStandardItem(nomeProduto);
    QStandardItem *newQuantidade = new QStandardItem(quantidadeProduto);
    QStandardItem *newDesc = new QStandardItem(descProduto);
    rowCount = model->rowCount();
    model->setItem(rowCount, 0, newNome);
    model->setItem(rowCount, 1, newQuantidade);
    model->setItem(rowCount, 2, newDesc);
    // começo escrita do arquivo
    QFile arquivo("../QEstoqueLoja/estoque.xml");
    if (arquivo.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QXmlStreamWriter xmlWriter(&arquivo);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartElement("produto");
        xmlWriter.writeStartElement("nome");
        xmlWriter.writeCharacters(addProduto.nome);
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("quantidade");
        xmlWriter.writeCharacters(QString::number(addProduto.quantidade));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("descricao");
        xmlWriter.writeCharacters(addProduto.descricao);
        xmlWriter.writeEndElement();
        xmlWriter.writeEndElement();
        xmlWriter.writeEndDocument();
        arquivo.flush();
        arquivo.close();
        ui->Ledit_Desc->clear();
        ui->Ledit_Nome->clear();
        ui->Ledit_Quantidade->clear();
        ui->Ledit_Nome->setFocus();
    } else {
        QMessageBox::warning(this,"ERRO", "Algo deu errado ao escrever no arquivo.");
    }
    // fim escrita do arquivo
}

