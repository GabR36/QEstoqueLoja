#include "janelaemailcontador.h"
#include "ui_janelaemailcontador.h"
#include <QMessageBox>

JanelaEmailContador::JanelaEmailContador(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::JanelaEmailContador)
{
    ui->setupUi(this);

    ui->Dedit_Fim->setMaximumDateTime(QDateTime::currentDateTime());
    ui->Dedit_Inicio->setMaximumDateTime(QDateTime::currentDateTime());

    QDate hoje = QDate::currentDate();
    ui->Dedit_Inicio->setDate(QDate(hoje.year(), hoje.month(), 1));
    ui->Dedit_Fim->setDate(hoje);

    atualizarContadores();
}

JanelaEmailContador::~JanelaEmailContador()
{
    delete ui;
}

void JanelaEmailContador::on_Dedit_Inicio_dateChanged(const QDate &date)
{
    Q_UNUSED(date)
    if (ui->Dedit_Inicio->date() > ui->Dedit_Fim->date())
        return;
    atualizarContadores();
}

void JanelaEmailContador::on_Dedit_Fim_dateChanged(const QDate &date)
{
    Q_UNUSED(date)
    if (ui->Dedit_Inicio->date() > ui->Dedit_Fim->date())
        return;
    atualizarContadores();
}

void JanelaEmailContador::atualizarContadores()
{
    QDateTime dtIni(ui->Dedit_Inicio->date().startOfDay());
    QDateTime dtFim(ui->Dedit_Fim->date().endOfDay());

    auto contagens = emailServ.buscarContagemContador(dtIni, dtFim);

    QString textoNfs = "NOTAS FISCAIS:\n";
    int totalNotas = 0;
    for (auto it = contagens.notasPorFinalidade.constBegin();
         it != contagens.notasPorFinalidade.constEnd(); ++it) {
        textoNfs += QString("• %1: %2\n").arg(it.key()).arg(it.value());
        totalNotas += it.value();
    }

    QString textoEventos = "\nEVENTOS FISCAIS:\n";
    int totalEventos = 0;
    for (auto it = contagens.eventosPorTipo.constBegin();
         it != contagens.eventosPorTipo.constEnd(); ++it) {
        textoEventos += QString("• %1: %2\n").arg(it.key()).arg(it.value());
        totalEventos += it.value();
    }

    QString resultado;
    resultado += QString("Total de Notas: %1\n").arg(totalNotas);
    resultado += QString("Total de Eventos: %1\n").arg(totalEventos);
    resultado += QString("Total Geral: %1").arg(totalNotas + totalEventos);

    ui->Lbl_TextoNotas->setText(textoNfs);
    ui->Lbl_TextoEventos->setText(textoEventos);
    ui->Lbl_DocsEncontrados->setText(resultado);
}

void JanelaEmailContador::on_pushButton_clicked()
{
    QDateTime dtIni(ui->Dedit_Inicio->date().startOfDay());
    QDateTime dtFim(ui->Dedit_Fim->date().endOfDay());

    auto res = emailServ.exportarEEnviarEmailContador(dtIni, dtFim);

    if (res.ok)
        QMessageBox::information(this, "E-mail enviado", res.msg);
    else
        QMessageBox::warning(this, "Erro ao enviar e-mail", res.msg);

    close();
}
