#include "email_service.h"
#include "../util/mailmanager.h"
#include <QDir>
#include <QFile>
#include <QLocale>

Email_service::Email_service(QObject *parent)
    : QObject{parent}
{
    confDTO = confServ.carregarTudo();
}

Email_service::Resultado Email_service::enviarEmailNFe(QString nomeCliente, QString emailCliente,
                                                       QString xmlPath, std::string pdfDanfe,
                                                       QDateTime dataVenda, QString nomeEmpresa)
{
    if(confDTO.emitNfFiscal && confDTO.tpAmbFiscal){
        try {
            auto mail = MailManager::instance().mail();

            QByteArray pdfBytes = QByteArray::fromBase64(
                QByteArray::fromStdString(pdfDanfe)
                );

            QString pdfPath =
                QDir::tempPath() + "/DANFE_" +
                QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") +
                ".pdf";

            QFile pdfFile(pdfPath);
            if (!pdfFile.open(QIODevice::WriteOnly)) {
                return {false, "Erro ao criar arquivo PDF DANFE temporário."};
            }
            pdfFile.write(pdfBytes);
            pdfFile.close();

            QLocale portugues(QLocale::Portuguese, QLocale::Brazil);
            QString dataFormatada = portugues.toString(
                dataVenda,
                "dddd, dd 'de' MMMM 'de' yyyy 'às' HH:mm"
                );

            QString corpo =
                "Olá " + nomeCliente + "\n\n"
                                       "Agradecemos por comprar da " + nomeEmpresa + "!\n"
                                "em anexo, você encontrará os arquivos referentes à "
                                "Nota Fiscal da compra de " + dataFormatada + ".\n\n"
                                  "Cordialmente,\n\n" + nomeEmpresa;

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
        catch (const std::exception& e) {
            return {false, QString("Erro ao enviar e-mail da NF-e: %1").arg(e.what())};
        }
    }else{
        return{true, "Email não enviado. Ambiente precisa estar em produção"};
    }

}
