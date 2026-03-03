#include "barcode_service.h"
#include <QByteArray>
#include <zint.h>
#include <QDebug>

Barcode_service::Barcode_service() {}

QImage Barcode_service::gerarCodigoBarras(const QString &codBar, QString *erro)
{
    if (codBar.trimmed().isEmpty()) {
        if (erro) *erro = "Código de barras vazio";
        return QImage();
    }

    QByteArray codBarBytes = codBar.toUtf8();
    const unsigned char* data =
        reinterpret_cast<const unsigned char*>(codBarBytes.constData());

    zint_symbol *barcode = ZBarcode_Create();
    if (!barcode) {
        if (erro) *erro = "Falha ao criar objeto Zint";
        return QImage();
    }

    barcode->symbology = BARCODE_CODE128;
    barcode->output_options = BOLD_TEXT;

    int error = ZBarcode_Encode(barcode, (unsigned char*)data, 0);
    if (error != 0) {
        if (erro) *erro = barcode->errtxt;
        ZBarcode_Delete(barcode);
        return QImage();
    }

    error = ZBarcode_Buffer(barcode, 0);
    if (error != 0) {
        if (erro) *erro = barcode->errtxt;
        ZBarcode_Delete(barcode);
        return QImage();
    }

    ZBarcode_Print(barcode, 0);
    ZBarcode_Delete(barcode);
#if defined(Q_OS_LINUX)
    QImage img("out.png");
#elif defined(Q_OS_WIN);
    QImage img("out.gif");
#endif
    if (img.isNull()) {
        if (erro) *erro = "Imagem do código de barras não foi gerada";
        return QImage();
    }

    return img;
}
