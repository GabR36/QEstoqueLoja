#ifndef ESCPOSCOMANDOS_H
#define ESCPOSCOMANDOS_H

#include <QByteArray>
#include <QImage>

class EscPosComandos
{
public:
    static QByteArray inicializar();

    static QByteArray alinharEsquerda();
    static QByteArray alinharCentro();
    static QByteArray alinharDireita();

    static QByteArray bold(bool enable);
    static QByteArray underline(bool enable);

    static QByteArray tamanhoFonte(int width, int height);

    static QByteArray feed(int linhas);

    static QByteArray cortar();

    static QByteArray imagem(const QImage &image);
};

#endif
