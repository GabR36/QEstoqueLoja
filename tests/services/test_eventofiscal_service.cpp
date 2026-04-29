#include "test_eventofiscal_service.h"
#include "../infra/databaseconnection_service.h"
#include <QFile>
#include <QSqlQuery>
#include <QDebug>

test_eventofiscal_service::test_eventofiscal_service(QObject *parent)
    : QObject{parent}
{}

void test_eventofiscal_service::init()
{
    QVERIFY(DatabaseConnection_service::open());
    QSqlQuery q(DatabaseConnection_service::db());

    q.exec("DELETE FROM eventos_fiscais");
}


void test_eventofiscal_service::enviarCienciaOp_sucesso()
{
    QString caminhoArquivo = ":/recursos/evento_cienciaop_sanitizado.txt";

    QFile file(caminhoArquivo);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(!retornoFake.isEmpty());

    EventoFiscal_service service;

    QString chave = "00000000000000000000000000000000000000000000";

    auto resultado = service.enviarCienciaOp(chave, retornoFake);

    QVERIFY(resultado.ok);
    QCOMPARE(resultado.erro, EventoFiscalErro::Nenhum);

    // valida se salvou no banco
    QVERIFY(DatabaseConnection_service::open());
    QSqlDatabase db = DatabaseConnection_service::db();

    QSqlQuery q(db);
    QVERIFY(q.exec("SELECT cstat, justificativa FROM eventos_fiscais ORDER BY id DESC LIMIT 1"));
    QVERIFY(q.next());

    QCOMPARE(q.value(0).toString(), QString("135"));
    QVERIFY(!q.value(1).toString().isEmpty());
}

void test_eventofiscal_service::enviarCienciaOp_deveFalharQuandoSefazRejeita()
{
    QString caminhoArquivo = ":/recursos/evento_cienciaop_sanitizado_erro.txt";

    QFile file(caminhoArquivo);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(!retornoFake.isEmpty());

    EventoFiscal_service service;

    QString chave = "00000000000000000000000000000000000000000000";

    auto resultado = service.enviarCienciaOp(chave, retornoFake);
    qDebug() << "MSG:" + resultado.msg;
    qDebug() << "OK:" + resultado.ok;
    QVERIFY(!resultado.ok);
    QCOMPARE(resultado.erro, EventoFiscalErro::EventoRecusadoSefaz);

    // QVERIFY(resultado.msg.contains("580"));
}

void test_eventofiscal_service::enviarCCE_sucesso()
{
    QString caminhoArquivo = ":/recursos/evento_cce_sanitizado.txt";

    QFile file(caminhoArquivo);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(!retornoFake.isEmpty());

    EventoFiscal_service service;

    QString chave = "00000000000000000000000000000000000000000000";
    int nseq = 1;
    QString correcao = "Texto de correcao valido com mais de 15 caracteres";

    auto resultado = service.enviarCCE(chave, nseq, correcao, retornoFake);

    QVERIFY(resultado.ok);
    QCOMPARE(resultado.erro, EventoFiscalErro::Nenhum);

    // valida banco
    QVERIFY(DatabaseConnection_service::open());
    QSqlQuery q(DatabaseConnection_service::db());

    QVERIFY(q.exec("SELECT cstat, justificativa FROM eventos_fiscais ORDER BY id DESC LIMIT 1"));
    QVERIFY(q.next());

    QCOMPARE(q.value(0).toString(), QString("135"));
    QVERIFY(q.value(1).toString().contains("Evento registrado"));
}

void test_eventofiscal_service::enviarCCE_deveFalharQuandoDuplicado()
{
    QString caminhoArquivo = ":/recursos/evento_cce_sanitizado_erro.txt";

    QFile file(caminhoArquivo);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(!retornoFake.isEmpty());

    EventoFiscal_service service;

    QString chave = "00000000000000000000000000000000000000000000";
    int nseq = 1;
    QString correcao = "Texto de correcao valido com mais de 15 caracteres";

    auto resultado = service.enviarCCE(chave, nseq, correcao, retornoFake);

    QVERIFY(!resultado.ok);
    QCOMPARE(resultado.erro, EventoFiscalErro::EventoRecusadoSefaz);

    QVERIFY(resultado.msg.contains("573"));
    QVERIFY(resultado.msg.contains("Duplicidade"));
}
