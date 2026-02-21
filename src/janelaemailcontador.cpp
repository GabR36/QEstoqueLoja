#include "janelaemailcontador.h"
#include "ui_janelaemailcontador.h"
#include <QSqlQuery>
#include <QDir>
#include <quazip/quazip.h>
#include <quazip/JlCompress.h>
#include "util/mailmanager.h"
#include "configuracao.h"
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>


JanelaEmailContador::JanelaEmailContador(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::JanelaEmailContador)
{
    ui->setupUi(this);
    contadorValues = Configuracao::get_All_Contador_Values();
    empresaValues = Configuracao::get_All_Empresa_Values();
    fiscalValues = Configuracao::get_All_Fiscal_Values();

    ui->Dedit_Fim->setMaximumDateTime(QDateTime::currentDateTime());
    ui->Dedit_Inicio->setMaximumDateTime(QDateTime::currentDateTime());

    db = QSqlDatabase::database();

    QDate hoje = QDate::currentDate();

    QDate inicioMes(hoje.year(), hoje.month(), 1);

    // QDate fimMes = inicioMes.addMonths(1).addDays(-1);

    ui->Dedit_Inicio->setDate(inicioMes);
    ui->Dedit_Fim->setDate(hoje);

    atualizarContadores();
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
        WHERE dhemi BETWEEN :ini AND :fim
        AND tp_amb = :tpamb
        GROUP BY finalidade
    )");
    
    qNotas.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    qNotas.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));
    qNotas.bindValue(":tpamb", fiscalValues.value("tp_amb"));
    qDebug() << fiscalValues;

    if (qNotas.exec()) {
        textoNfs += "NOTAS FISCAIS:\n";
        while (qNotas.next()) {
            QString finalidade = qNotas.value(0).toString();
            int qtd = qNotas.value(1).toInt();
            totalNotas += qtd;

            textoNfs += QString("• %1: %2\n").arg(finalidade).arg(qtd);
        }
    } else {
        qDebug() << "ERRO query email";
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
    QDateTime dtFim(ui->Dedit_Fim->date().endOfDay());

    QString textoNfs;
    QString textoEventos;

    QSqlQuery qNotas(db);
    qNotas.prepare(R"(
        SELECT finalidade, xml_path
        FROM notas_fiscais
        WHERE dhemi BETWEEN :ini AND :fim
        AND tp_amb = :tpamb
    )");
    qNotas.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    qNotas.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));
    qNotas.bindValue(":tpamb", fiscalValues.value("tp_amb"));

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
    // zipar o diretorio

    QString zipPath = baseDir + ".zip";

    bool ok = JlCompress::compressDir(zipPath, baseDir);

    QString pdfPath = baseDir + "/relatorio_NotasEmitidas.pdf";
    gerarResumoPdf(pdfPath, dtIni, dtFim);

    if (!ok) {
        qDebug() << "Erro ao criar ZIP";
    } else {
        qDebug() << "ZIP criado em:" << zipPath;
    }

    enviarEmailContador(zipPath, dtIni.date(), dtFim.date(), pdfPath);

    // remover os arquivos no tmp

    QDir dirExp(baseDir);
    if (dirExp.exists()) {
        if (!dirExp.removeRecursively()) {
            qDebug() << "Falha ao remover diretório:" << dirExp;
        }
    }

    if (QFile::exists(zipPath)) {
        if (!QFile::remove(zipPath)) {
            qDebug() << "Falha ao remover o arquivo:" << zipPath;
        }
    }

    close();
}

void JanelaEmailContador::enviarEmailContador(QString zip, QDate dtIni, QDate dtFim, QString pdfPath) {
    try {
        auto mail = MailManager::instance().mail();

        QString corpo;

        corpo = "Olá " + contadorValues.value("contador_nome") + ",\n\n"
            "Em anexo, você encontrará os documentos fiscais referente ao período entre "
                + dtIni.toString("dd/MM/yyyy") + " e "
                + dtFim.toString("dd/MM/yyyy") + ".";

        mail->Limpar();
        mail->LimparAnexos();
        mail->AddCorpoAlternativo(corpo.toStdString());
        mail->SetAssunto("Documentos Fiscais de " + empresaValues.value("nome_empresa").toStdString());
        mail->AddDestinatario(contadorValues.value("contador_email").toStdString());
        mail->AddAnexo(zip.toStdString(), "XMLs compactados");
        mail->AddAnexo(pdfPath.toStdString(), "Resumo das Notas");

        mail->Enviar();

        QMessageBox::information(this, "E-mail enviado com sucesso", "O e-mail foi enviado.");
    }
    catch (const std::exception& e) {
        QMessageBox::warning(this, "Erro ao enviar e-mail", "O e-mail não foi enviado.");
    }
}

void JanelaEmailContador::gerarResumoPdf(const QString &filePath, QDateTime dtIni, QDateTime dtFim)
{
    if (!db.isOpen()) {
        if (!db.open()) return;
    }

    QPdfWriter pdf(filePath);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);

    QPainter painter(&pdf);

    int margemEsquerda = 20;   // 👉 ajuste horizontal aqui
    int margemTopo     = 120;
    int espacamentoLinha = 60;  // 👉 AUMENTE ISSO para mais espaço
    int y = margemTopo;

    // 🔹 Título
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    QString titulo = QString("NF Autorizadas no período de %1 até %2")
                         .arg(dtIni.date().toString("dd/MM/yyyy"))
                         .arg(dtFim.date().toString("dd/MM/yyyy"));

    painter.drawText(margemEsquerda, y, titulo);
    y += 80;

    // 🔹 Cabeçalho
    painter.setFont(QFont("Arial", 11, QFont::Bold));

    painter.drawText(margemEsquerda, y, "Número");
    painter.drawText(margemEsquerda + 250, y, "Emissão");
    painter.drawText(margemEsquerda + 500, y, "Chave");
    painter.drawText(margemEsquerda + 1700, y, "Valor Total");
    painter.drawText(margemEsquerda + 2000, y, "Situação");

    y += espacamentoLinha;

    painter.setFont(QFont("Arial", 11));

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT nnf, dhemi, chnfe, valor_total, finalidade, cstat
        FROM notas_fiscais
        WHERE dhemi BETWEEN :ini AND :fim
        AND tp_amb = :tpamb
        AND finalidade != 'ENTRADA EXTERNA'
        ORDER BY dhemi
    )");

    query.bindValue(":ini", dtIni.toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":fim", dtFim.toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":tpamb", fiscalValues.value("tp_amb"));

    int totalRegistros = 0;
    double totalGeral = 0.0;

    if (query.exec()) {
        while (query.next()) {

            // 🔹 quebra de página correta
            if (y + espacamentoLinha > pdf.height() - margemTopo) {
                pdf.newPage();
                y = margemTopo;
            }

            QString numero = query.value("nnf").toString();
            QDate data = query.value("dhemi").toDateTime().date();
            QString chave = query.value("chnfe").toString();
            double valor = query.value("valor_total").toDouble();
            QString finalidade = query.value("finalidade").toString();
            int cstat = query.value("cstat").toInt();

            QString situacao;
            double valorParaTotal = valor;

            if (cstat == 135) {
                situacao = "CANCELADO";
                valorParaTotal = 0;  // subtrai do total
            }
            else if (finalidade == "DEVOLUCAO") {
                situacao = "DEVOLUÇÃO";
                valorParaTotal = -valor;  // subtrai do total
            }
            else {
                situacao = "AUTORIZADA";
            }


            painter.drawText(margemEsquerda, y, numero);
            painter.drawText(margemEsquerda + 250, y, data.toString("dd/MM/yyyy"));
            painter.drawText(margemEsquerda + 500, y, chave);
            painter.drawText(margemEsquerda + 1700, y,
                             QLocale(QLocale::Portuguese, QLocale::Brazil)
                                 .toCurrencyString(valorParaTotal));

            painter.drawText(margemEsquerda + 2000, y, situacao);
            // linha separadora
            painter.drawLine(margemEsquerda, y + 15,
                             pdf.width() - margemEsquerda, y + 15);

            totalRegistros++;
            totalGeral += valorParaTotal;

            y += espacamentoLinha;
        }
    }

    y += 60;
    painter.setFont(QFont("Arial", 12, QFont::Bold));

    painter.drawText(margemEsquerda, y,
                     QString("Total de Registros: %1").arg(totalRegistros));

    y += 40;

    painter.drawText(margemEsquerda, y,
                     QString("Valor Total Geral: %1")
                         .arg(QLocale(QLocale::Portuguese, QLocale::Brazil)
                                  .toCurrencyString(totalGeral)));

    painter.end();
}
