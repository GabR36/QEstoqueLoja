#include <QtTest>
#include "test_produto_service.h"
#include "services/Produto_service.h"
#include "../db/test_db_factory.h"
#include <QLocale>
#include <QSqlQuery>
#include "infra/databaseconnection_service.h"

void TestProdutoService::inserir_produto_ok()
{
    Produto_Service service;


    ProdutoDTO p;
    p.quantidade = 10.00;
    p.descricao = "Produto Teste";
    p.preco = 5.50;
    p.codigoBarras = "7898765430018";
    p.nf = 0;
    p.uCom = "UN";
    p.precoFornecedor = 4.00;
    p.percentLucro = 20.00;
    p.ncm = "00000000";
    p.cest = "";
    p.aliquotaIcms = 32.50;
    p.csosn = "102";
    p.pis = "49";

    auto r = service.inserir(p);

    QVERIFY(r.ok == true);
    if(!DatabaseConnection_service::open()){
        qDebug() << "banco nao aberto teste produtos";
    }
    QSqlQuery query(db);
    query.exec("SELECT * FROM produtos ORDER BY id DESC");
    QString desc, codbarras, nf, ucom, ncm, cest, csosn, pis;
    double preco, precoforn, quant, aliquota, percentlucro;

    while(query.next()){
        quant = query.value("quantidade").toDouble();
        desc = query.value("descricao").toString();
        preco = query.value("preco").toDouble();
        codbarras = query.value("codigo_barras").toString();
        nf = query.value("nf").toString();
        ucom = query.value("un_comercial").toString();
        precoforn = query.value("preco_fornecedor").toDouble();
        percentlucro = query.value("porcent_lucro").toFloat();
        ncm = query.value("ncm").toString();
        cest = query.value("cest").toString();
        aliquota = query.value("aliquota_imposto").toFloat();
        csosn = query.value("csosn").toString();
        pis = query.value("pis").toString();
    }
    db.close();
    QCOMPARE(quant, 10);
    QCOMPARE(desc, "Produto Teste");
    QCOMPARE(preco, 5.5);
    QCOMPARE(codbarras, "7898765430018");
    QCOMPARE(nf, "0");
    QCOMPARE(ucom, "UN");
    QCOMPARE(precoforn, 4);
    QCOMPARE(percentlucro, 20);
    QCOMPARE(ncm, "00000000");
    QCOMPARE(cest, "");
    QCOMPARE(aliquota, 32.5);
    QCOMPARE(csosn, "102");
    QCOMPARE(pis, "49");

}

void TestProdutoService::init()
{
    db = DatabaseConnection_service::db();
}



void TestProdutoService::inserir_produto_errado()
{
    Produto_Service service;

    ProdutoDTO p;
    p.quantidade = 0;
    p.descricao = "";
    p.preco = -5.50;
    p.codigoBarras = "";
    p.nf = 0;
    p.uCom = "UN";
    p.precoFornecedor = -4.00;
    p.percentLucro = -20.00;
    p.ncm = "12313";
    p.cest = "";
    p.aliquotaIcms = -2.9;
    p.csosn = "102";
    p.pis = "49";

    auto r = service.inserir(p);

    QVERIFY(!r.ok);

    QSqlQuery query(db);
    if(!DatabaseConnection_service::open()){
        qDebug() << "banco nao aberto teste produtos";
    }
    QVERIFY(query.exec("SELECT COUNT(*) FROM produtos"));
    QVERIFY(query.next());

    int count = query.value(0).toInt();
    db.close();

    QCOMPARE(count, 0);
}


void TestProdutoService::erro_codigo_barras_existente()
{
    Produto_Service service;

    ProdutoDTO p;
    p.quantidade = 1;
    p.descricao = "Produto 1";
    p.preco = 10;
    p.codigoBarras = "ABC";
    p.nf = false;
    p.uCom = "UN";
    p.precoFornecedor = 5;
    p.percentLucro = 10;
    p.ncm = "";
    p.cest = "";
    p.aliquotaIcms = 18;
    p.csosn = "102";
    p.pis = "1";

    service.inserir(p);
    auto r2 = service.inserir(p);

    QVERIFY(!r2.ok);
    QCOMPARE(r2.erro, ProdutoErro::CodigoBarrasExistente);
}

void TestProdutoService::erro_preco_invalido()
{
    Produto_Service service;

    ProdutoDTO p;
    p.quantidade = 1;
    p.descricao = "Produto";
    p.preco = -2.909090;
    p.codigoBarras = "111";
    p.nf = false;
    p.uCom = "UN";
    p.precoFornecedor = 5;
    p.percentLucro = 10;
    p.ncm = "";
    p.cest = "";
    p.aliquotaIcms = 18;
    p.csosn = "102";
    p.pis = "1";

    auto r = service.inserir(p);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, ProdutoErro::PrecoInvalido);
}

void TestProdutoService::erro_quantidade_invalida()
{
    Produto_Service service;

    ProdutoDTO p;
    p.quantidade = 0;
    p.descricao = "Produto";
    p.preco = 10;
    p.codigoBarras = "222";
    p.nf = false;
    p.uCom = "UN";
    p.precoFornecedor = 5;
    p.percentLucro = 10;
    p.ncm = "";
    p.cest = "";
    p.aliquotaIcms = 18;
    p.csosn = "102";
    p.pis = "1";

    auto r = service.inserir(p);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, ProdutoErro::QuantidadeInvalida);
}

void TestProdutoService::erro_ncm_nf()
{
    Produto_Service service;

    ProdutoDTO p;
    p.quantidade = 1;
    p.descricao = "Produto";
    p.preco = 10;
    p.codigoBarras = "333";
    p.nf = true;
    p.uCom = "UN";
    p.precoFornecedor = 5;
    p.percentLucro = 10;
    p.ncm = ""; // inválido para NF
    p.cest = "";
    p.aliquotaIcms = 18;
    p.csosn = "102";
    p.pis = "1";

    auto r = service.inserir(p);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, ProdutoErro::NcmInvalido);
}

void TestProdutoService::deletar_produto_ok()
{
    Produto_Service service;

    // inserir produto
    ProdutoDTO p;
    p.quantidade = 5;
    p.descricao = "Produto Delete";
    p.preco = 10;
    p.codigoBarras = "DEL123";
    p.nf = false;
    p.uCom = "UN";
    p.precoFornecedor = 6;
    p.percentLucro = 20;
    p.ncm = "";
    p.cest = "";
    p.aliquotaIcms = 18;
    p.csosn = "102";
    p.pis = "1";

    auto r = service.inserir(p);
    QVERIFY(r.ok);


    if(!DatabaseConnection_service::open()){
        qDebug() << "banco nao aberto teste produtos";
    }
    // pegar id
    QSqlQuery q(db);
    QVERIFY(q.exec("SELECT id FROM produtos WHERE codigo_barras = 'DEL123'"));
    QVERIFY(q.next());
    QString id = q.value(0).toString();

    // deletar
    auto rd = service.deletar(id);
    QVERIFY(rd.ok);

    // validar no banco
    if(!DatabaseConnection_service::open()){
        qDebug() << "banco nao aberto teste produtos";
    }
    QSqlQuery q2(db);
    QVERIFY(q2.exec("SELECT COUNT(*) FROM produtos WHERE id = " + id));
    QVERIFY(q2.next());

    int count = q2.value(0).toInt();
    QCOMPARE(count, 0);
}

void TestProdutoService::deletar_produto_inexistente()
{
    // db = TestDbFactory::create();
    Produto_Service service;

    auto r = service.deletar("999999"); // id inexistente
    QVERIFY(!r.ok);
    QCOMPARE(r.erro, ProdutoErro::ErroBanco);
}


void TestProdutoService::alterar_produto_ok()
{
    Produto_Service service;

    // inserir produto original
    ProdutoDTO p;
    p.quantidade = 10;
    p.descricao = "Produto Original";
    p.preco = 10;
    p.codigoBarras = "ALT001";
    p.nf = false;
    p.uCom = "UN";
    p.precoFornecedor = 5;
    p.percentLucro = 10;
    p.ncm = "";
    p.cest = "";
    p.aliquotaIcms = 18;
    p.csosn = "102";
    p.pis = "1";

    QVERIFY(service.inserir(p).ok);
    if(!DatabaseConnection_service::open()){
        qDebug() << "banco nao aberto teste produtos";
    }
    // pegar id
    QSqlQuery q(db);
    QVERIFY(q.exec("SELECT id FROM produtos WHERE codigo_barras = 'ALT001'"));
    QVERIFY(q.next());
    QString id = q.value(0).toString();

    // novo produto (alterado)
    ProdutoDTO novo;
    novo.quantidade = 20;
    novo.descricao = "Produto Alterado";
    novo.preco = 25;
    novo.codigoBarras = "ALT001"; // mesmo código
    novo.nf = false;
    novo.uCom = "CX";
    novo.precoFornecedor = 15;
    novo.percentLucro = 30;
    novo.ncm = "";
    novo.cest = "";
    novo.aliquotaIcms = 12;
    novo.csosn = "500";
    novo.pis = "2";

    auto r = service.alterarVerificarCodigoBarras(novo, "ALT001", id);
    QVERIFY(r.ok);
    if(!DatabaseConnection_service::open()){
        qDebug() << "banco nao aberto teste produtos";
    }
    // validar banco
    QSqlQuery q2(db);
    QVERIFY(q2.exec("SELECT * FROM produtos WHERE id = " + id));
    QVERIFY(q2.next());

    QCOMPARE(q2.value("descricao").toString(), QString("PRODUTO ALTERADO"));
    QCOMPARE(q2.value("quantidade").toDouble(), 20.0);
    QCOMPARE(q2.value("preco").toDouble(), 25.0);
    QCOMPARE(q2.value("un_comercial").toString(), QString("CX"));
    QCOMPARE(q2.value("preco_fornecedor").toDouble(), 15.0);
    QCOMPARE(q2.value("porcent_lucro").toDouble(), 30.0);
    QCOMPARE(q2.value("aliquota_imposto").toDouble(), 12.0);
    QCOMPARE(q2.value("csosn").toString(), QString("500"));
}

void TestProdutoService::cleanup()
{
    if(!DatabaseConnection_service::open()){
        qDebug() << "banco nao aberto teste produtos";
    }
    QSqlQuery q(db);
    q.exec("DELETE FROM produtos");
}


