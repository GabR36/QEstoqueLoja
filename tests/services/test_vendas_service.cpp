#include <QtTest>
#include "test_vendas_service.h"
#include "services/vendas_service.h"
#include "services/Produto_service.h"
#include "../db/test_db_factory.h"
#include <QSqlQuery>
#include "infra/databaseconnection_service.h"

namespace {

qlonglong inserirProdutoTeste(QSqlDatabase &db, const QString &codBarras)
{
    Produto_Service ps;
    ProdutoDTO p;
    p.quantidade      = 50;
    p.descricao       = "Produto Teste Venda";
    p.preco           = 20.00;
    p.codigoBarras    = codBarras;
    p.nf              = false;
    p.uCom            = "UN";
    p.precoFornecedor = 10.00;
    p.percentLucro    = 20.00;
    p.ncm             = "";
    p.cest            = "";
    p.aliquotaIcms    = 18;
    p.csosn           = "102";
    p.pis             = "49";
    ps.inserir(p);

    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.prepare("SELECT id FROM produtos WHERE codigo_barras = :cod");
    q.bindValue(":cod", codBarras);
    q.exec();
    q.next();
    qlonglong id = q.value(0).toLongLong();
    db.close();
    return id;
}

Vendas_service::ResultadoInsercaoRN inserirVendaTeste(qlonglong idProd, double quantidade = 2)
{
    VendasDTO v;
    v.clienteNome    = "Consumidor";
    v.dataHora       = "2026-03-16 10:00:00";
    v.total          = quantidade * 20.00;
    v.formaPagamento = "Dinheiro";
    v.valorRecebido  = quantidade * 20.00;
    v.troco          = 0;
    v.taxa           = 0;
    v.valorFinal     = quantidade * 20.00;
    v.desconto       = 0;
    v.estaPago       = true;
    v.idCliente      = 1;

    ProdutoVendidoDTO pv;
    pv.idProduto    = idProd;
    pv.idVenda      = 0;
    pv.quantidade   = quantidade;
    pv.precoVendido = 20.00;

    Vendas_service service;
    return service.inserirVendaRegraDeNegocio(v, {pv});
}

} // namespace

void TestVendasService::init()
{
    db = DatabaseConnection_service::db();
}

void TestVendasService::cleanup()
{
    if (!DatabaseConnection_service::open())
        return;
    QSqlQuery q(db);
    q.exec("DELETE FROM produtos_vendidos");
    q.exec("DELETE FROM vendas2");
    q.exec("DELETE FROM produtos");
    db.close();
}

void TestVendasService::inserir_venda_ok()
{
    qlonglong idProd = inserirProdutoTeste(db, "VT001");
    QVERIFY(idProd > 0);

    auto r = inserirVendaTeste(idProd, 2);

    QVERIFY(r.ok);
    QVERIFY(r.idVendaInserida > 0);

    // verifica venda no banco
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec(QString("SELECT total, valor_recebido FROM vendas2 WHERE id = %1").arg(r.idVendaInserida));
    QVERIFY(q.next());
    QCOMPARE(q.value("total").toDouble(), 40.00);
    QCOMPARE(q.value("valor_recebido").toDouble(), 40.00);

    // verifica que a quantidade do produto diminuiu (50 - 2 = 48)
    QSqlQuery q2(db);
    q2.exec(QString("SELECT quantidade FROM produtos WHERE id = %1").arg(idProd));
    QVERIFY(q2.next());
    QCOMPARE(q2.value(0).toDouble(), 48.0);

    // verifica produto vendido salvo
    QSqlQuery q3(db);
    q3.exec(QString("SELECT COUNT(*) FROM produtos_vendidos WHERE id_venda = %1").arg(r.idVendaInserida));
    QVERIFY(q3.next());
    QCOMPARE(q3.value(0).toInt(), 1);

    db.close();
}

void TestVendasService::erro_desconto_maior_que_total()
{
    qlonglong idProd = inserirProdutoTeste(db, "VT002");

    VendasDTO v;
    v.clienteNome    = "Consumidor";
    v.dataHora       = "2026-03-16 10:00:00";
    v.total          = 30.00;
    v.formaPagamento = "Dinheiro";
    v.valorRecebido  = 30.00;
    v.troco          = 0;
    v.taxa           = 0;
    v.valorFinal     = 30.00;
    v.desconto       = 50.00; // maior que o total
    v.estaPago       = true;
    v.idCliente      = 1;

    ProdutoVendidoDTO pv;
    pv.idProduto    = idProd;
    pv.quantidade   = 1;
    pv.precoVendido = 30.00;

    Vendas_service service;
    auto r = service.inserirVendaRegraDeNegocio(v, {pv});

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, VendasErro::QuebraDeRegra);
}

void TestVendasService::erro_prazo_sem_cliente()
{
    qlonglong idProd = inserirProdutoTeste(db, "VT003");

    VendasDTO v;
    v.clienteNome    = "";
    v.dataHora       = "2026-03-16 10:00:00";
    v.total          = 50.00;
    v.formaPagamento = "Prazo";
    v.valorRecebido  = 0;
    v.troco          = 0;
    v.taxa           = 0;
    v.valorFinal     = 50.00;
    v.desconto       = 0;
    v.estaPago       = false;
    v.idCliente      = 1; // <= 1, não especificado

    ProdutoVendidoDTO pv;
    pv.idProduto    = idProd;
    pv.quantidade   = 1;
    pv.precoVendido = 50.00;

    Vendas_service service;
    auto r = service.inserirVendaRegraDeNegocio(v, {pv});

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, VendasErro::QuebraDeRegra);
}

void TestVendasService::deletar_venda_ok()
{
    qlonglong idProd = inserirProdutoTeste(db, "VT004");
    auto ri = inserirVendaTeste(idProd, 3);
    QVERIFY(ri.ok);

    Vendas_service service;
    auto rd = service.deletarVendaRegraNegocio(ri.idVendaInserida, false);
    QVERIFY(rd.ok);

    DatabaseConnection_service::open();

    // venda removida
    QSqlQuery q(db);
    q.exec(QString("SELECT COUNT(*) FROM vendas2 WHERE id = %1").arg(ri.idVendaInserida));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 0);

    // produtos_vendidos removidos
    QSqlQuery q2(db);
    q2.exec(QString("SELECT COUNT(*) FROM produtos_vendidos WHERE id_venda = %1").arg(ri.idVendaInserida));
    QVERIFY(q2.next());
    QCOMPARE(q2.value(0).toInt(), 0);

    // quantidade do produto restaurada (50 - 3 + 3 = 50)
    QSqlQuery q3(db);
    q3.exec(QString("SELECT quantidade FROM produtos WHERE id = %1").arg(idProd));
    QVERIFY(q3.next());
    QCOMPARE(q3.value(0).toDouble(), 50.0);

    db.close();
}

void TestVendasService::devolver_produto_ok()
{
    qlonglong idProd = inserirProdutoTeste(db, "VT005");
    auto ri = inserirVendaTeste(idProd, 2);
    QVERIFY(ri.ok);

    // obter id do produto vendido
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec(QString("SELECT id FROM produtos_vendidos WHERE id_venda = %1").arg(ri.idVendaInserida));
    QVERIFY(q.next());
    qlonglong idProdVend = q.value(0).toLongLong();
    db.close();

    Vendas_service service;
    auto rd = service.devolverProdutoRegraNegocio(idProdVend, ri.idVendaInserida);
    QVERIFY(rd.ok);

    DatabaseConnection_service::open();

    // quantidade do produto restaurada (50 - 2 + 2 = 50)
    QSqlQuery q2(db);
    q2.exec(QString("SELECT quantidade FROM produtos WHERE id = %1").arg(idProd));
    QVERIFY(q2.next());
    QCOMPARE(q2.value(0).toDouble(), 50.0);

    // total da venda atualizado (40 - 40 = 0)
    QSqlQuery q3(db);
    q3.exec(QString("SELECT total FROM vendas2 WHERE id = %1").arg(ri.idVendaInserida));
    QVERIFY(q3.next());
    QCOMPARE(q3.value(0).toDouble(), 0.0);

    db.close();
}

void TestVendasService::calcular_resumo()
{
    qlonglong idProd = inserirProdutoTeste(db, "VT006");
    auto ri = inserirVendaTeste(idProd, 3);
    QVERIFY(ri.ok);

    Vendas_service service;
    ResumoVendasDTO resumo = service.calcularResumo("2026-01-01", "2026-12-31", false, 0);

    QVERIFY(resumo.total >= 60.0);
    QVERIFY(resumo.quantidade >= 1);
}
