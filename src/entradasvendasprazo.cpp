#include "entradasvendasprazo.h"
#include "ui_entradasvendasprazo.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include "pagamentoaprazo.h"
#include "delegatehora.h"

EntradasVendasPrazo::EntradasVendasPrazo(QWidget *parent, QString id_venda)
    : QDialog(parent)
    , ui(new Ui::EntradasVendasPrazo)
{
    ui->setupUi(this);
    idVenda = id_venda;
    ui->label->setText(idVenda);

    // validador
    QDoubleValidator *validador = new QDoubleValidator(0.0, 9999.99, 2);
    ui->ledit_AddValor->setValidator(validador);

    VendasDTO venda = vendasServ.getVenda(idVenda.toLongLong());
    valor_Venda   = static_cast<float>(venda.valorFinal);
    valorVenda    = portugues.toString(valor_Venda, 'f', 2);
    dataHoraVenda = portugues.toString(
        QDateTime::fromString(venda.dataHora, Qt::ISODate), "dd/MM/yyyy");
    clienteVenda  = venda.clienteNome;

    atualizarTabelaPag();
    ui->tview_Entradas->setModel(modeloEntradas);
    modeloEntradas->setHeaderData(0, Qt::Horizontal, tr("Total"));
    modeloEntradas->setHeaderData(1, Qt::Horizontal, tr("Data e Hora"));
    modeloEntradas->setHeaderData(2, Qt::Horizontal, tr("Forma de Pagamento"));
    modeloEntradas->setHeaderData(3, Qt::Horizontal, tr("Valor Final"));
    modeloEntradas->setHeaderData(4, Qt::Horizontal, tr("Troco"));
    modeloEntradas->setHeaderData(5, Qt::Horizontal, tr("Taxa"));
    modeloEntradas->setHeaderData(6, Qt::Horizontal, tr("Valor Recebido"));
    modeloEntradas->setHeaderData(7, Qt::Horizontal, tr("Desconto"));
    ui->tview_Entradas->setColumnWidth(0,100);
    ui->tview_Entradas->setColumnWidth(1,150);
    ui->tview_Entradas->setColumnWidth(2,160);

    DelegateHora *delegateHora = new DelegateHora(this);
    ui->tview_Entradas->setItemDelegateForColumn(1, delegateHora);

    ui->label_2->setText("Valor Inicial da Venda: " + valorVenda);
    ui->label_3->setText("Data Inicial da Venda: "+ dataHoraVenda);
    ui->label_4->setText("Cliente: "+clienteVenda);
    QModelIndex firstIndex = modeloEntradas->index(0, 0);
    ui->tview_Entradas->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
    // --icons
    QIcon deletar(":/QEstoqueLOja/amarok-cart-remove.svg");
    ui->btn_DeletarEntrada->setIcon(deletar);
}

EntradasVendasPrazo::~EntradasVendasPrazo()
{
    delete ui;
}

void EntradasVendasPrazo::atualizarTabelaPag(){
    QList<EntradaVendaDTO> entradas = entradaServ.getEntradasFromVenda(idVenda.toLongLong());
    float valorDevido = valor_Venda;
    for (const auto &e : entradas)
        valorDevido -= static_cast<float>(e.total);

    entradaServ.listarEntradasVenda(modeloEntradas, idVenda.toLongLong());

    vendasServ.atualizarEstaPago(idVenda.toLongLong(), valorDevido <= 0);

    ui->label_5->setText("Devendo: R$" + portugues.toString(valorDevido, 'f', 2));
    if (valorDevido > 0) {
        ui->label_5->setStyleSheet("color: red");
    } else {
        ui->label_5->setStyleSheet("color: green");
    }
    valorDevidoGlobal = valorDevido;

    if(valorDevido <= 0) {
        ui->btn_AddValor->setEnabled(false);
    } else {
        ui->btn_AddValor->setEnabled(true);
    }
}

void EntradasVendasPrazo::on_btn_AddValor_clicked()
{
    float valorInseridoIngles = ui->ledit_AddValor->text().toFloat();
    float valorInserido = portugues.toFloat(ui->ledit_AddValor->text());
    // se tiver algo a dever e o valor informado for menor ou igual ao valor devido
    if ((valorDevidoGlobal > 0) && (valorInseridoIngles <= valorDevidoGlobal)) {

        pagamentoAPrazo *pgmntPrazo= new pagamentoAPrazo(idVenda, portugues.toString(valorInserido, 'f', 2), clienteVenda,
                                                          portugues.toString(QDateTime::currentDateTime(), "yyyy-MM-dd hh:mm:ss"));
        connect(pgmntPrazo, &pagamentoAPrazo::pagamentoPrazoConcluido, this,
                &EntradasVendasPrazo::entradaConcluida);

       connect(pgmntPrazo, &QObject::destroyed, this, &EntradasVendasPrazo::onPgmntFechado);

        pgmntPrazo->show();
    }
}

void EntradasVendasPrazo::onPgmntFechado(){
    qDebug() << "atualizarTAbelaPag?";
    atualizarTabelaPag();
    ui->ledit_AddValor->clear();
}

void EntradasVendasPrazo::on_btn_DeletarEntrada_clicked()
{
    if(ui->tview_Entradas->currentIndex().isValid()){
        // obter id selecionado
        QItemSelectionModel *selectionModel = ui->tview_Entradas->selectionModel();
        QModelIndex selectedIndex = selectionModel->selectedIndexes().first();
        QVariant idVariant = ui->tview_Entradas->model()->data(ui->tview_Entradas->model()->index(selectedIndex.row(), 8));
        QVariant valorVariant = ui->tview_Entradas->model()->data(ui->tview_Entradas->model()->index(selectedIndex.row(), 0));

        idEntradaSelec = idVariant.toString();
        QString valorEntradaSelec = valorVariant.toString();

        // Cria uma mensagem de confirmação
        QMessageBox::StandardButton resposta;
        resposta = QMessageBox::question(
            nullptr,
            "Confirmação",
            "Tem certeza que deseja excluir a entrada:\n\n"
            "id: " + idEntradaSelec + "\n"
            "Valor total: " + valorEntradaSelec,
            QMessageBox::Yes | QMessageBox::No
            );
        // Verifica a resposta do usuário
        if (resposta == QMessageBox::Yes) {
            auto res = entradaServ.deletarEntradaPorId(idEntradaSelec.toLongLong());
            if (!res.ok) {
                qDebug() << "Erro ao deletar entrada:" << res.msg;
            }
            atualizarTabelaPag();
            emit entradaConcluida();
        } else {
            // O usuário escolheu não deletar o produto
            qDebug() << "A exclusão da entrada foi cancelada.";
        }
    } else {
        QMessageBox::warning(this,"Erro","Selecione uma entrada antes de tentar deletar!");
    }
}
