#ifndef PRINTUTIL_H
#define PRINTUTIL_H

#include <QObject>

class PrintUtil : public QObject
{
    Q_OBJECT
public:
    explicit PrintUtil(QObject *parent = nullptr);

    static bool imprimirEtiquetas(QWidget *parent, int quant, const QImage &barcodeImage, const QString &desc, double preco, QString *erro);
private:
signals:
};

#endif // PRINTUTIL_H
