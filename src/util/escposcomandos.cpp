#include "escposcomandos.h"
#include <QImage>

// EscPosComandos::EscPosComandos()
// {}
QByteArray EscPosComandos::inicializar()
{
    return QByteArray("\x1B\x40", 2);
}

QByteArray EscPosComandos::imagem(const QImage &image)
{
    QImage img = image.convertToFormat(QImage::Format_Mono);

    img = img.scaled(
        560,
        img.height(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    int width = img.width();
    int height = img.height();
    int bytesPerLine = (width + 7) / 8;
    QByteArray data;

    data.append(char(0x1D));
    data.append(char(0x76));
    data.append(char(0x30));
    data.append(char(0x00));

    data.append(char(bytesPerLine & 0xFF));
    data.append(char((bytesPerLine >> 8) & 0xFF));

    data.append(char(height & 0xFF));
    data.append(char((height >> 8) & 0xFF));

    for (int y = 0; y < height; y++)
    {
        for (int xb = 0; xb < bytesPerLine; xb++)
        {
            uchar byte = 0;

            for (int bit = 0; bit < 8; bit++)
            {
                int x = xb * 8 + bit;

                if (x >= width)
                    continue;

                QColor c = img.pixelColor(x, y);

                if (c.black() > 127)
                    byte |= (1 << (7 - bit));
            }

            data.append(byte);
        }
    }
    return data;
}

QByteArray EscPosComandos::cortar()
{
    return QByteArray("\x1D\x56\x00", 3);
}

QByteArray EscPosComandos::alinharCentro()
{
    return QByteArray("\x1B\x61\x01", 3);
}

QByteArray EscPosComandos::alinharEsquerda()
{
    return QByteArray("\x1B\x61\x00", 3);
}

QByteArray EscPosComandos::bold(bool on)
{
    QByteArray cmd;
    cmd.append('\x1B');
    cmd.append('\x45');
    cmd.append(on ? '\x01' : '\x00');

    return cmd;
}

QByteArray EscPosComandos::feed(int linhas)
{
    QByteArray cmd;

    cmd.append('\x1B');
    cmd.append('\x64');
    cmd.append(char(linhas));

    return cmd;
}
