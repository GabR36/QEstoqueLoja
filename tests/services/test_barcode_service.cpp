#include "test_barcode_service.h"
#include <QImage>

test_barcode_service::test_barcode_service(QObject *parent)
    : QObject{parent}
{}

void test_barcode_service::codigoValido_deveGerarImagem()
{
    QString erro;
    QImage img = Barcode_service::gerarCodigoBarras("7891234567890", &erro);

    QVERIFY2(!img.isNull(), "Imagem deveria ser válida");
    QVERIFY2(erro.isEmpty(), "Erro deveria estar vazio");
}

void test_barcode_service::codigoVazio_deveFalhar()
{
    QString erro;
    QImage img = Barcode_service::gerarCodigoBarras("", &erro);

    QVERIFY2(img.isNull(), "Imagem deveria ser nula");
    QVERIFY2(!erro.isEmpty(), "Erro deveria estar preenchido");
    QCOMPARE(erro, QString("Código de barras vazio"));
}

