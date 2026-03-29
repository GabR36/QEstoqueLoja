#include "codigobarrasutil.h"
#include <qrandom.h>

CodigoBarrasUtil::CodigoBarrasUtil(QObject *parent)
    : QObject{parent}
{}


QString CodigoBarrasUtil::gerarNumeroCodigoBarrasNaoFiscal()
{
    QSet<QString> generatedNumbers;
    QString number;
    do {
        number = QString("3562%1").arg(QRandomGenerator::global()->bounded(100000), 5, 10, QChar('0'));
    } while (generatedNumbers.contains(number));

    generatedNumbers.insert(number);
    // saveGeneratedNumber(number);

    return number;
}
