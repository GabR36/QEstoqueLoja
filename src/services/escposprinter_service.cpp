#include "escposprinter_service.h"
#include <QProcess>
#include <QLocale>
#include "../util/escposcomandos.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <winspool.h>
#endif

EscPosPrinter_service::EscPosPrinter_service(QObject *parent)
: QObject(parent)
{

}

bool EscPosPrinter_service::imprimirTeste(const QString &printerName, QString *erro)
{
    QByteArray dados;

    dados += EscPosComandos::inicializar();

    dados += EscPosComandos::alinharCentro();
    dados += "TESTE ESC/POS\n";
    dados += "ELGIN I9\n\n";

    dados += EscPosComandos::alinharEsquerda();
    dados += "Produto Teste\n";
    dados += "Preco: R$ 10,00\n\n";

    dados += "\n\n\n\n";
    dados += EscPosComandos::cortar();
    auto r1 = imprimirRaw(printerName, dados);
    return r1.ok;
}

EscPosPrinter_service::Resultado EscPosPrinter_service::imprimirRaw(
    const QString &printerName,
    const QByteArray &dados)
{

#ifdef Q_OS_LINUX

    QProcess process;

    process.start("lp", {"-d", printerName, "-o", "raw"});

    if (!process.waitForStarted())
        return {false, "Não foi possível iniciar o comando lp."};

    process.write(dados);
    process.closeWriteChannel();

    if (!process.waitForFinished())
    {
        return {false, "Falha ao enviar dados para a impressora."};
    }
    if (process.exitCode() != 0)
        return {false, process.readAllStandardError()};

    return {true, ""};

#elif defined(Q_OS_WIN)


    HANDLE hPrinter = nullptr;

    if (!OpenPrinterW((LPWSTR)printerName.utf16(), &hPrinter, nullptr))
    {
        return {false, "Não foi possível abrir a impressora."};
    }

    DOC_INFO_1W docInfo;
    docInfo.pDocName = (LPWSTR)L"ESC/POS";
    docInfo.pOutputFile = nullptr;
    docInfo.pDatatype = (LPWSTR)L"RAW";

    if (!StartDocPrinterW(hPrinter, 1, (LPBYTE)&docInfo))
    {
        ClosePrinter(hPrinter);

        return {false, "Erro ao iniciar documento."};
    }

    StartPagePrinter(hPrinter);

    DWORD written = 0;

    BOOL ok = WritePrinter(
        hPrinter,
        (LPVOID)dados.constData(),
        dados.size(),
        &written);

    EndPagePrinter(hPrinter);
    EndDocPrinter(hPrinter);
    ClosePrinter(hPrinter);

    if (!ok || written != (DWORD)dados.size())
    {
        return {false, "Erro ao enviar dados RAW."};

    }

    return {true, ""};

#else

    Q_UNUSED(printerName)
    Q_UNUSED(dados)
    return {false, "Sistema operacional ainda não suportado."};

#endif
}


EscPosPrinter_service::Resultado EscPosPrinter_service::imprimirEtiquetas(
    const QString &printerName,
    int quantidade,
    const QImage &barcodeImage,
    const QString &descricao,
    double preco)
{
    if (barcodeImage.isNull())
    {
        return {false, "Imagem de código de barras inválida"};
    }

    QLocale pt(QLocale::Portuguese, QLocale::Brazil);

    QByteArray dados;
    dados += EscPosComandos::inicializar();

    for (int i = 0; i < quantidade; ++i)
    {
        dados += EscPosComandos::alinharEsquerda();

        dados += descricao.toLatin1();
        dados += "\n";

        dados += EscPosComandos::bold(true);

        QString texto = QString("Preco: R$ %1")
                            .arg(pt.toString(preco,'f',2));

        dados += texto.toLatin1();
        dados += "\n";

        dados += EscPosComandos::bold(false);

        dados += EscPosComandos::imagem(barcodeImage);

        dados += "\n";

        dados += EscPosComandos::feed(4);

        if (i != quantidade - 1)
            dados += EscPosComandos::cortar();
    }

    dados += EscPosComandos::cortar();

    return imprimirRaw(printerName, dados);
}


