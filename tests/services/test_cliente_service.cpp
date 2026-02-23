#include "test_cliente_service.h"
#include "services/cliente_service.h"
#include "../db/test_db_factory.h"
#include <QSqlQuery>
#include <QTest>

test_cliente_service::test_cliente_service(QObject *parent)
    : QObject{parent}
{

}


void test_cliente_service::init()
{
    db = TestDbFactory::create();
}

void test_cliente_service::inserir_cliente_nome_vazio()
{
    Cliente_service service;

    ClienteDTO c;
    c.nome = "";
    c.cpf = "12345678901";
    c.ehPf = true;

    auto r = service.inserirCliente(c);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, ClienteErro::CampoVazio);
}


void test_cliente_service::erro_cpf_duplicado()
{
    Cliente_service service;

    ClienteDTO c;
    c.nome = "Cliente 1";
    c.cpf = "11111111111";
    c.ehPf = true;

    QVERIFY(service.inserirCliente(c).ok);

    auto r2 = service.inserirCliente(c);

    QVERIFY(!r2.ok);
    QCOMPARE(r2.erro, ClienteErro::InsercaoInvalida);
}

void test_cliente_service::erro_email_invalido()
{
    Cliente_service service;

    ClienteDTO c;
    c.nome = "Cliente";
    c.cpf = "22222222222";
    c.ehPf = true;
    c.email = "email_invalido";

    auto r = service.inserirCliente(c);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, ClienteErro::QuebraDeRegra);
}

void test_cliente_service::erro_telefone_invalido()
{
    Cliente_service service;

    ClienteDTO c;
    c.nome = "Cliente";
    c.cpf = "33333333333";
    c.ehPf = true;
    c.telefone = "abc";

    auto r = service.inserirCliente(c);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, ClienteErro::QuebraDeRegra);
}

void test_cliente_service::deletar_cliente_ok()
{
    Cliente_service service;

    ClienteDTO c;
    c.nome = "Cliente Delete";
    c.cpf = "44444444444";
    c.ehPf = true;

    QVERIFY(service.inserirCliente(c).ok);

    QSqlQuery q(db);
    QVERIFY(q.exec("SELECT id FROM clientes WHERE cpf = '44444444444'"));
    QVERIFY(q.next());

    qlonglong id = q.value(0).toLongLong();

    auto r = service.deletarCliente(id);
    QVERIFY(r.ok);

    QSqlQuery q2(db);
    QVERIFY(q2.exec("SELECT COUNT(*) FROM clientes WHERE id = " + QString::number(id)));
    QVERIFY(q2.next());

    QCOMPARE(q2.value(0).toInt(), 0);
}

void test_cliente_service::deletar_cliente_protegido()
{
    Cliente_service service;

    auto r = service.deletarCliente(1);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, ClienteErro::QuebraDeRegra);
}


void test_cliente_service::alterar_cliente_ok()
{
    Cliente_service service;

    ClienteDTO c;
    c.nome = "Original";
    c.cpf = "55555555555";
    c.ehPf = true;

    QVERIFY(service.inserirCliente(c).ok);

    QSqlQuery q(db);
    QVERIFY(q.exec("SELECT id FROM clientes WHERE cpf = '55555555555'"));
    QVERIFY(q.next());

    qlonglong id = q.value(0).toLongLong();

    ClienteDTO novo;
    novo.nome = "Alterado";
    novo.cpf = "55555555555";
    novo.ehPf = true;

    auto r = service.updateCliente(id, novo);
    QVERIFY(r.ok);

    QSqlQuery q2(db);
    QVERIFY(q2.exec("SELECT nome FROM clientes WHERE id = " + QString::number(id)));
    QVERIFY(q2.next());

    QCOMPARE(q2.value(0).toString(), QString("Alterado"));
}


void test_cliente_service::alterar_cliente_invalido()
{
    Cliente_service service;

    ClienteDTO c;
    c.nome = "";
    c.cpf = "66666666666";
    c.ehPf = true;

    auto r = service.updateCliente(1, c);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, ClienteErro::CampoVazio);
}

void test_cliente_service::cleanup()
{
    QSqlQuery q(db);
    q.exec("DELETE FROM clientes");
}


