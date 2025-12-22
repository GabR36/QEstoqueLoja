#include "janelaemailcontador.h"
#include "ui_janelaemailcontador.h"
#include <QSqlQuery>
#include <QDir>
#include "util/ziputil.h"
#include <quazip/quazip.h>
#include <quazip/JlCompress.h>
#include "util/mailmanager.h"
#include "configuracao.h"


JanelaEmailContador::JanelaEmailContador(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::JanelaEmailContador)
{
    ui->setupUi(this);
    db = QSqlDatabase::database();

    QDate hoje = QDate::currentDate();

    QDate inicioMes(hoje.year(), hoje.month(), 1);

    QDate fimMes = inicioMes.addMonths(1).addDays(-1);

    ui->Dedit_Inicio->setDate(inicioMes);
    ui->Dedit_Fim->setDate(fimMes);

    atualizarContadores();
    contadorValues = Configuracao::get_All_Contador_Values();
    empresaValues = Configuracao::get_All_Empresa_Values();
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
    QDateTime dtFim(ui->Dedit_Fim->date(), QTime::currentTime());

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



void JanelaEmailContador::on_pushButton_clicked()
{
    QString baseDir = QDir::tempPath() + "/exportacao_xml";

    QDir dir;
    dir.mkpath(baseDir);
    dir.mkpath(baseDir + "/notas_fiscais");
    dir.mkpath(baseDir + "/eventos_fiscais");

    if (!db.isOpen()){
        if(!db.open()){
            qDebug() << "bd nao abriu atualizarCOntadores()";
            return;
        }
    }

    QDateTime dtIni(ui->Dedit_Inicio->date().startOfDay());
    QDateTime dtFim(ui->Dedit_Fim->date(), QTime::currentTime());

    QString textoNfs;
    QString textoEventos;

    QSqlQuery qNotas(db);
    qNotas.prepare(R"(
        SELECT finalidade, xml_path
        FROM notas_fiscais
        WHERE atualizado_em BETWEEN :ini AND :fim
    )");
    qNotas.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    qNotas.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));

    if (qNotas.exec()) {
        textoNfs += "NOTAS FISCAIS:\n";
        while (qNotas.next()) {
            QString finalidade = qNotas.value("finalidade").toString();
            QString path = qNotas.value("xml_path").toString();

            QString destinoDir = baseDir + "/notas_fiscais/" + finalidade;
            QDir().mkpath(destinoDir);

            QString nomeArquivo = QFileInfo(path).fileName();
            QString destinoFinal = destinoDir + "/" + nomeArquivo;

            if (!QFile::copy(path, destinoFinal)) {
                qDebug() << "Erro ao copiar:" << path;
            }
        }
    }

    QSqlQuery qEventos(db);
    qEventos.prepare(R"(
        SELECT tipo_evento, xml_path
        FROM eventos_fiscais
        WHERE atualizado_em BETWEEN :ini AND :fim
    )");
    qEventos.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    qEventos.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));


    if (qEventos.exec()) {
        textoEventos += "\nEVENTOS FISCAIS:\n";
        while (qEventos.next()) {
            QString tipo = qEventos.value("tipo_evento").toString();
            QString path = qEventos.value("xml_path").toString();

            QString destinoDir = baseDir + "/eventos_fiscais/" + tipo;
            QDir().mkpath(destinoDir);

            QString nomeArquivo = QFileInfo(path).fileName();
            QString destinoFinal = destinoDir + "/" + nomeArquivo;

            if (!QFile::copy(path, destinoFinal)) {
                qDebug() << "Erro ao copiar:" << path;
            }
        }
    }

    QString zipPath = baseDir + ".zip";

    bool ok = JlCompress::compressDir(zipPath, baseDir);

    if (!ok) {
        qDebug() << "Erro ao criar ZIP";
    } else {
        qDebug() << "ZIP criado em:" << zipPath;
    }

    enviarEmailContador(zipPath, dtIni, dtFim);
}

void JanelaEmailContador::enviarEmailContador(QString zip, QDateTime dtIni, QDateTime dtFim) {
    try {
        auto mail = MailManager::instance().mail();

        QString corpo;

        corpo = "Olá " + contadorValues.value("contador_nome") + ",\n\n"
            "Em anexo, você encontrará os documentos fiscais referente ao período entre "
                + dtIni.toString("dd/MM/yyyy HH:mm:ss") + " e "
                + dtFim.toString("dd/MM/yyyy HH:mm:ss") + ".";

        mail->Limpar();
        mail->LimparAnexos();
        mail->AddCorpoAlternativo(corpo.toStdString());
        mail->SetAssunto("Documentos Fiscais de " + empresaValues.value("nome_empresa").toStdString());
        mail->AddDestinatario(contadorValues.value("contador_email").toStdString());
        mail->AddAnexo(zip.toStdString(), "XMLs compactados");

        mail->Enviar();

        qDebug() << "Email enviado com sucesso";
    }
    catch (const std::exception& e) {
        qDebug() << "Erro ao enviar email:" << e.what();
    }
}
