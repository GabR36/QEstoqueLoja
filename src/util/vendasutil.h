#ifndef VENDASUTIL_H
#define VENDASUTIL_H

#include <QObject>


class VendasUtil : public QObject
{
    Q_OBJECT
public:
    enum class VendasFormaPagamento {
        Dinheiro,
        NaoSei,
        Credito,
        Debito,
        Pix,
        Prazo,
        Nenhuma
    };

    explicit VendasUtil(QObject *parent = nullptr);

signals:
};

#endif // VENDASUTIL_H
