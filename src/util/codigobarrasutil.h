#ifndef CODIGOBARRASUTIL_H
#define CODIGOBARRASUTIL_H

#include <QObject>
#include <qset.h>

class CodigoBarrasUtil : public QObject
{
    Q_OBJECT
public:
    explicit CodigoBarrasUtil(QObject *parent = nullptr);
    static QString gerarNumeroCodigoBarrasNaoFiscal();
private:


signals:
};

#endif // CODIGOBARRASUTIL_H
