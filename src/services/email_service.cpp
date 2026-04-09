#include "email_service.h"
#include "../util/mailmanager.h"
#include "../util/pdfexporter.h"
#include "../util/ziputil.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QDebug>

Email_service::Email_service(QObject *parent)
    : QObject{parent}
{
    confDTO = confServ.carregarTudo();
}

QString Email_service::footer()
{
    return "\n\n"
           "---\n"
           "QEstoqueLoja - Sistema de Gestao de Estoque e Vendas Gratuito "
           "e de Código Aberto.\nSaiba Mais: https://qestoqueloja.mentolog.top";
}

Email_service::Resultado Email_service::enviarEmailNFe(QString nomeCliente, QString emailCliente,
                                                       QString xmlPath, std::string pdfDanfe,
                                                       QDateTime dataVenda, QString nomeEmpresa)
{
    if (!confDTO.emitNfFiscal || !confDTO.tpAmbFiscal)
        return {true, "Email não enviado. Ambiente precisa estar em produção"};

    try {
        auto mail = MailManager::instance().mail();

        QByteArray pdfBytes = QByteArray::fromBase64(QByteArray::fromStdString(pdfDanfe));
        QString pdfPath = QDir::tempPath() + "/DANFE_"
                          + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".pdf";

        QFile pdfFile(pdfPath);
        if (!pdfFile.open(QIODevice::WriteOnly))
            return {false, "Erro ao criar arquivo PDF DANFE temporário."};
        pdfFile.write(pdfBytes);
        pdfFile.close();

        QLocale portugues(QLocale::Portuguese, QLocale::Brazil);
        QString dataFormatada = portugues.toString(dataVenda, "dddd, dd 'de' MMMM 'de' yyyy 'às' HH:mm");

        QString corpo = "Olá " + nomeCliente + "\n\n"
                        "Agradecemos por comprar da " + nomeEmpresa + "!\n"
                        "Em anexo, você encontrará os arquivos referentes à "
                        "Nota Fiscal da compra de " + dataFormatada + ".\n\n"
                        "Cordialmente,\n\n" + nomeEmpresa
                        + footer();

        mail->Limpar();
        mail->LimparAnexos();
        mail->AddCorpoAlternativo(corpo.toStdString());
        mail->SetAssunto("Nota Fiscal Eletrônica de " + nomeEmpresa.toStdString());
        mail->AddDestinatario(emailCliente.toStdString());
        mail->AddAnexo(xmlPath.toStdString(), "XML NFe", 0);
        mail->AddAnexo(pdfPath.toStdString(), "DANFE (PDF)", 0);

        mail->Enviar();

        return {true, "E-mail da NF-e enviado com sucesso."};
    }
    catch (const std::exception &e) {
        return {false, QString("Erro ao enviar e-mail da NF-e: %1").arg(e.what())};
    }
}

Email_service::ContagemContador Email_service::buscarContagemContador(QDateTime dtIni, QDateTime dtFim)
{
    ContagemContador result;
    result.notasPorFinalidade = notaServ.contarPorFinalidade(dtIni, dtFim, confDTO.tpAmbFiscal);
    result.eventosPorTipo     = eventoServ.contarPorTipo(dtIni, dtFim);
    return result;
}

Email_service::Resultado Email_service::exportarEEnviarEmailContador(QDateTime dtIni, QDateTime dtFim)
{
    QString baseDir = QDir::tempPath() + "/exportacao_xml";
    QDir().mkpath(baseDir + "/notas_fiscais");
    QDir().mkpath(baseDir + "/eventos_fiscais");

    for (const auto &par : notaServ.buscarXmlsPorPeriodo(dtIni, dtFim, confDTO.tpAmbFiscal)) {
        QString destinoDir = baseDir + "/notas_fiscais/" + par.first;
        QDir().mkpath(destinoDir);
        if (!QFile::copy(par.second, destinoDir + "/" + QFileInfo(par.second).fileName()))
            qDebug() << "Erro ao copiar NF XML:" << par.second;
    }

    for (const auto &par : eventoServ.buscarXmlsPorPeriodo(dtIni, dtFim)) {
        QString destinoDir = baseDir + "/eventos_fiscais/" + par.first;
        QDir().mkpath(destinoDir);
        if (!QFile::copy(par.second, destinoDir + "/" + QFileInfo(par.second).fileName()))
            qDebug() << "Erro ao copiar Evento XML:" << par.second;
    }

    QString zipPath = baseDir + ".zip";
    if (!ZipUtil::comprimirDiretorio(baseDir, zipPath))
        qDebug() << "Erro ao criar ZIP";

    QString pdfPath = baseDir + "/relatorio_NotasEmitidas.pdf";
    PDFexporter::exportarResumoContadorPdf(pdfPath, dtIni, dtFim,
                                           notaServ.buscarPorPeriodo(dtIni, dtFim, confDTO.tpAmbFiscal));

    try {
        auto mail = MailManager::instance().mail();

        QString corpo = "Olá " + confDTO.nomeContador + ",\n\n"
                        "Em anexo, você encontrará os documentos fiscais referente ao período entre "
                        + dtIni.date().toString("dd/MM/yyyy") + " e "
                        + dtFim.date().toString("dd/MM/yyyy") + "."
                        + footer();

        mail->Limpar();
        mail->LimparAnexos();
        mail->AddCorpoAlternativo(corpo.toStdString());
        mail->SetAssunto("Documentos Fiscais de " + confDTO.nomeEmpresa.toStdString());
        mail->AddDestinatario(confDTO.emailContador.toStdString());
        mail->AddAnexo(zipPath.toStdString(), "XMLs compactados");
        mail->AddAnexo(pdfPath.toStdString(), "Resumo das Notas");

        mail->Enviar();

        QDir(baseDir).removeRecursively();
        QFile::remove(zipPath);

        return {true, "E-mail enviado ao contador com sucesso."};
    }
    catch (const std::exception &e) {
        QDir(baseDir).removeRecursively();
        QFile::remove(zipPath);

        return {false, QString("Erro ao enviar e-mail ao contador: %1").arg(e.what())};
    }
}
