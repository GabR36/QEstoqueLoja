#include "test_manifestadordfe.h"
#include <QFile>
#include <QSqlQuery>
#include <QDebug>

test_manifestadordfe::test_manifestadordfe(QObject *parent)
    : QObject{parent}
{}

void test_manifestadordfe::verificar_resumosalvo_retornoForcado()
{

    QString caminhoArquivo = ":/recursos/consultaDFE_sanitizada.txt";

    QFile file(caminhoArquivo);

    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Erro ao abrir arquivo de retorno fake");

    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(!retornoFake.isEmpty());


    ManifestadorDFe manifestador;
    manifestador.setRetornoForcado(retornoFake);
    manifestador.consultarEManifestar();

    QVERIFY(DatabaseConnection_service::open());
    db = DatabaseConnection_service::db();
    //validar dfe_info

    // qDebug() << "Connection name:" << db.connectionName();
    // qDebug() << "Is open:" << db.isOpen();
    QSqlQuery checkUlt(db);
    QVERIFY(checkUlt.exec("SELECT ult_nsu FROM dfe_info WHERE identificacao = 'consulta_resumo'"));

    QVERIFY(checkUlt.next());

    QString ultNsuSalvo = checkUlt.value(0).toString();

    QCOMPARE(ultNsuSalvo, QString("986"));



    //validar resumo salvo

    QSqlQuery checkResumo(db);
    QVERIFY(checkResumo.exec(
        "SELECT finalidade, valor_total, xml_path FROM notas_fiscais WHERE cnpjemit = '00000000000191'"
        ));

    QVERIFY(checkResumo.next());

    QString finalidade = checkResumo.value(0).toString();
    QString valor_total = checkResumo.value(1).toString();
    QString xml_path = checkResumo.value(2).toString();

    QCOMPARE(finalidade, QString("resNFe"));
    QCOMPARE(valor_total, QString("648.04"));
    QCOMPARE(xml_path, QString("/tmp/teste/resNFe.xml"));

}

void test_manifestadordfe::verificar_NotaFiscalEntradaSalvo_retornoForcado()
{
    QString caminhoArquivo = ":/recursos/consultaDFE_sanitizada.txt";

    QFile file(caminhoArquivo);

    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Erro ao abrir arquivo de retorno fake");

    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(!retornoFake.isEmpty());


    ManifestadorDFe manifestador;
    manifestador.setRetornoForcado(retornoFake);
    manifestador.consultarEBaixarXML();

    QVERIFY(DatabaseConnection_service::open());
    db = DatabaseConnection_service::db();
    //validar dfe_info

    // qDebug() << "Connection name:" << db.connectionName();
    // qDebug() << "Is open:" << db.isOpen();
    QSqlQuery checkUlt(db);
    QVERIFY(checkUlt.exec("SELECT ult_nsu FROM dfe_info WHERE identificacao = 'consulta_xml'"));

    QVERIFY(checkUlt.next());

    QString ultNsuSalvo = checkUlt.value(0).toString();

    QCOMPARE(ultNsuSalvo, QString("972"));



    //validar resumo salvo

    QSqlQuery checkResumo(db);
    QVERIFY(checkResumo.exec(
        "SELECT cstat, modelo, valor_total, finalidade, chnfe, dhemi FROM notas_fiscais WHERE cnpjemit = '00000000000191'"
        ));

    QVERIFY(checkResumo.next());

    QString cstat = checkResumo.value(0).toString();
    QString modelo = checkResumo.value(1).toString();
    QString valor_total = checkResumo.value(2).toString();
    QString finalidade = checkResumo.value(3).toString();
    QString chnfe = checkResumo.value(4).toString();
    // QString xml_path = checkResumo.value(2).toString();
    QString dhemi = checkResumo.value(5).toString();


    QCOMPARE(cstat, QString("100"));
    QCOMPARE(modelo, QString("55"));
    QCOMPARE(finalidade, QString("ENTRADA EXTERNA"));
    QCOMPARE(chnfe, QString("99999999999999999999550010000000011000000000"));
    // QCOMPARE(xml_path, QString("/tmp/teste/resNFe.xml"));
    QCOMPARE(valor_total, QString("648.04"));
    QCOMPARE(dhemi, QString("2026-02-03 14:38:10"));


}


void test_manifestadordfe::cleanupTestCase()
{
    QSqlQuery q(db);
    q.exec("DELETE FROM notas_fiscais");
    q.exec("DELETE FROM eventos_fiscais");
    q.exec("DELETE FROM produtos_nota");


}

