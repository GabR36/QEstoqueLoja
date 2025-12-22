#include "janelaemailcontador.h"
#include "ui_janelaemailcontador.h"
#include <QSqlQuery>

JanelaEmailContador::JanelaEmailContador(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::JanelaEmailContador)
{
    ui->setupUi(this);
    db = QSqlDatabase::database();
}

JanelaEmailContador::~JanelaEmailContador()
{
    delete ui;
}

void JanelaEmailContador::on_Dedit_Inicio_dateChanged(const QDate &date)
{
    if (ui->Dedit_Inicio->date() > ui->Dedit_Fim->date())
        return;
    atualizarContadores();
}


void JanelaEmailContador::on_Dedit_Fim_dateChanged(const QDate &date)
{
    if (ui->Dedit_Inicio->date() > ui->Dedit_Fim->date())
        return;
    atualizarContadores();
}

void JanelaEmailContador::atualizarContadores()
{
    if (!db.isOpen()){
        if(!db.open()){
            qDebug() << "bd nao abriu atualizarCOntadores()";
            return;
        }
    }


    QDateTime dtIni(ui->Dedit_Inicio->date().startOfDay());
    QDateTime dtFim(ui->Dedit_Fim->date().endOfDay());

    int totalNotas = 0;
    int totalEventos = 0;

    QString texto;
    QString textoNfs;
    QString textoEventos;
    QString resultado;

    // =========================
    // CONTAGEM NOTAS FISCAIS
    // =========================
    QSqlQuery qNotas(db);
    qNotas.prepare(R"(
        SELECT finalidade, COUNT(*)
        FROM notas_fiscais
        WHERE atualizado_em BETWEEN :ini AND :fim
        GROUP BY finalidade
    )");

    qNotas.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    qNotas.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));

    if (qNotas.exec()) {
        textoNfs += "NOTAS FISCAIS:\n";
        while (qNotas.next()) {
            QString finalidade = qNotas.value(0).toString();
            int qtd = qNotas.value(1).toInt();
            totalNotas += qtd;

            textoNfs += QString("• %1: %2\n").arg(finalidade).arg(qtd);
        }
    }

    // =========================
    // CONTAGEM EVENTOS FISCAIS
    // =========================
    QSqlQuery qEventos(db);
    qEventos.prepare(R"(
        SELECT tipo_evento, COUNT(*)
        FROM eventos_fiscais
        WHERE atualizado_em BETWEEN :ini AND :fim
        GROUP BY tipo_evento
    )");

    qEventos.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    qEventos.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));


    if (qEventos.exec()) {
        textoEventos += "\nEVENTOS FISCAIS:\n";
        while (qEventos.next()) {
            QString tipo = qEventos.value(0).toString();
            int qtd = qEventos.value(1).toInt();
            totalEventos += qtd;

            textoEventos += QString("• %1: %2\n").arg(tipo).arg(qtd);
        }
    }



    // =========================
    // RESUMO FINAL
    // =========================
    resultado += QString("Total de Notas: %1\n").arg(totalNotas);
    resultado += QString("Total de Eventos: %1\n").arg(totalEventos);
    resultado += QString("Total Geral: %1").arg(totalNotas + totalEventos);
    ui->Lbl_TextoNotas->setText(textoNfs);
    ui->Lbl_TextoEventos->setText(textoEventos);
    ui->Lbl_DocsEncontrados->setText(resultado);
}


