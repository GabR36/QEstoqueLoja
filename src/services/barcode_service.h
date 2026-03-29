#ifndef BARCODE_SERVICE_H
#define BARCODE_SERVICE_H
#include <QImage>

class Barcode_service
{
public:
    Barcode_service();
    static QImage gerarCodigoBarras(const QString &codBar, QString *erro);
};

#endif // BARCODE_SERVICE_H
