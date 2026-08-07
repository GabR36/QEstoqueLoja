#ifndef ESCPOSPRINTER_SERVICE_H
#define ESCPOSPRINTER_SERVICE_H
#include <QObject>
#include <QByteArray>
#include <QImage>

class EscPosPrinter_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        QString msg;
    };
    explicit EscPosPrinter_service(QObject *parent = nullptr);
    bool imprimirTeste(const QString &printerName, QString *erro = nullptr);

    EscPosPrinter_service::Resultado imprimirRaw(const QString &printerName,
                     const QByteArray &dados);
    EscPosPrinter_service::Resultado imprimirEtiquetas(const QString &printerName, int quantidade,
                                                       const QImage &barcodeImage, const QString &descricao,
                                                       double preco);
private:
signals:
};

#endif // ESCPOSPRINTER_SERVICE_H
