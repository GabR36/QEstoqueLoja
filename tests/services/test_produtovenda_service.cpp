#include <QtTest>
#include "test_produtovenda_service.h"
#include "services/produtovenda_service.h"
#include "services/vendas_service.h"
#include "services/Produto_service.h"
#include "../db/test_db_factory.h"
#include <QSqlQuery>
#include "infra/databaseconnection_service.h"
#include "dto/Produto_dto.h"

namespace {

qlonglong inserirProdutoTeste(QSqlDatabase &db, const QString &codBarras)
{
    Produto_Service ps;
    ProdutoDTO p;
    p.quantidade      = 50;
    p.descricao       = "Produto Teste ProdVenda";
    p.preco           = 15.00;
    p.codigoBarras    = codBarras;
    p.nf              = false;
    p.uCom            = "UN";
    p.precoFornecedor = 8.00;
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

qlonglong inserirVendaTeste(qlonglong idProd)
{
    VendasDTO v;
    v.clienteNome    = "Consumidor";
    v.dataHora       = "2026-03-16 10:00:00";
    v.total          = 15.00;
    v.formaPagamento = "Dinheiro";
    v.valorRecebido  = 15.00;
    v.troco          = 0;
    v.taxa           = 0;
    v.valorFinal     = 15.00;
    v.desconto       = 0;
    v.estaPago       = true;
    v.idCliente      = 1;

    ProdutoVendidoDTO pv;
    pv.idProduto    = idProd;
    pv.idVenda      = 0;
    pv.quantidade   = 1;
    pv.precoVendido = 15.00;

    Vendas_service service;
    auto r = service.inserirVendaRegraDeNegocio(v, {pv});
    return r.ok ? r.idVendaInserida : -1;
}

} // namespace

void TestProdutoVendaService::init()
{
    db = DatabaseConnection_service::db();
}

void TestProdutoVendaService::cleanup()
{
    if (!DatabaseConnection_service::open())
        return;
    QSqlQuery q(db);
    q.exec("DELETE FROM produtos_vendidos");
    q.exec("DELETE FROM vendas2");
    q.exec("DELETE FROM produtos");
    db.close();
}

void TestProdutoVendaService::inserir_produto_vendido_ok()
{
    qlonglong idProd = inserirProdutoTeste(db, "PV001");
    QVERIFY(idProd > 0);

    ProdutoVendidoDTO pv;
    pv.idProduto    = idProd;
    pv.idVenda      = 999; // id de venda fictício (sem FK constraint no SQLite por padrão)
    pv.quantidade   = 3;
    pv.precoVendido = 15.00;

    ProdutoVenda_service service;
    auto r = service.inserir(pv);
    QVERIFY(r.ok);

    // verifica no banco
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec("SELECT id_produto, quantidade, preco_vendido, adicionado_em, atualizado_em, emitido_nf "
           "FROM produtos_vendidos WHERE id_venda = 999");
    QVERIFY(q.next());
    QCOMPARE(q.value("id_produto").toLongLong(), idProd);
    QCOMPARE(q.value("quantidade").toDouble(), 3.0);
    QCOMPARE(q.value("preco_vendido").toDouble(), 15.00);
    QVERIFY(!q.value("adicionado_em").toString().isEmpty());
    QVERIFY(!q.value("atualizado_em").toString().isEmpty());
    QCOMPARE(q.value("emitido_nf").toInt(), 0);
    db.close();
}

void TestProdutoVendaService::deletar_produto_vendido_ok()
{
    qlonglong idProd = inserirProdutoTeste(db, "PV002");
    qlonglong idVenda = inserirVendaTeste(idProd);
    QVERIFY(idVenda > 0);

    // obtém id do produto_vendido
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec(QString("SELECT id FROM produtos_vendidos WHERE id_venda = %1").arg(idVenda));
    QVERIFY(q.next());
    qlonglong idProdVend = q.value(0).toLongLong();
    db.close();

    ProdutoVenda_service service;
    auto r = service.deletarProdutoVendido(idProdVend);
    QVERIFY(r.ok);

    // verifica que foi removido
    DatabaseConnection_service::open();
    QSqlQuery q2(db);
    q2.exec(QString("SELECT COUNT(*) FROM produtos_vendidos WHERE id = %1").arg(idProdVend));
    QVERIFY(q2.next());
    QCOMPARE(q2.value(0).toInt(), 0);
    db.close();
}

void TestProdutoVendaService::deletar_por_idvenda()
{
    qlonglong idProd1 = inserirProdutoTeste(db, "PV003");
    qlonglong idProd2 = inserirProdutoTeste(db, "PV004");
    QVERIFY(idProd1 > 0);
    QVERIFY(idProd2 > 0);

    // insere venda com dois produtos
    VendasDTO v;
    v.clienteNome    = "Consumidor";
    v.dataHora       = "2026-03-16 10:00:00";
    v.total          = 30.00;
    v.formaPagamento = "Dinheiro";
    v.valorRecebido  = 30.00;
    v.troco          = 0;
    v.taxa           = 0;
    v.valorFinal     = 30.00;
    v.desconto       = 0;
    v.estaPago       = true;
    v.idCliente      = 1;

    ProdutoVendidoDTO pv1;
    pv1.idProduto    = idProd1;
    pv1.idVenda      = 0;
    pv1.quantidade   = 1;
    pv1.precoVendido = 15.00;

    ProdutoVendidoDTO pv2;
    pv2.idProduto    = idProd2;
    pv2.idVenda      = 0;
    pv2.quantidade   = 1;
    pv2.precoVendido = 15.00;

    Vendas_service vendaService;
    auto ri = vendaService.inserirVendaRegraDeNegocio(v, {pv1, pv2});
    QVERIFY(ri.ok);

    // verifica que dois produtos foram inseridos
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec(QString("SELECT COUNT(*) FROM produtos_vendidos WHERE id_venda = %1").arg(ri.idVendaInserida));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 2);
    db.close();

    ProdutoVenda_service service;
    auto rd = service.deletarProdutosVendidosPorIdVenda(ri.idVendaInserida);
    QVERIFY(rd.ok);

    // verifica que todos foram removidos
    DatabaseConnection_service::open();
    QSqlQuery q2(db);
    q2.exec(QString("SELECT COUNT(*) FROM produtos_vendidos WHERE id_venda = %1").arg(ri.idVendaInserida));
    QVERIFY(q2.next());
    QCOMPARE(q2.value(0).toInt(), 0);
    db.close();
}

void TestProdutoVendaService::tem_apenas_um_produto_true()
{
    qlonglong idProd = inserirProdutoTeste(db, "PV005");
    qlonglong idVenda = inserirVendaTeste(idProd);
    QVERIFY(idVenda > 0);

    ProdutoVenda_service service;
    QVERIFY(service.temApenasUmProduto(idVenda));
}

void TestProdutoVendaService::marcar_emitido_nf_emitir_todos()
{
    // produto nf=false
    qlonglong idProdSemNf = inserirProdutoTeste(db, "NF_EMIT_ALL_A");
    // produto nf=true
    Produto_Service ps;
    ProdutoDTO pNf;
    pNf.quantidade      = 10;
    pNf.descricao       = "Produto NF True";
    pNf.preco           = 20.00;
    pNf.codigoBarras    = "NF_EMIT_ALL_B";
    pNf.nf              = true;
    pNf.uCom            = "UN";
    pNf.precoFornecedor = 10.00;
    pNf.percentLucro    = 20.00;
    pNf.ncm             = "12345678";
    pNf.cest            = "";
    pNf.aliquotaIcms    = 18;
    pNf.csosn           = "102";
    pNf.pis             = "49";
    ps.inserir(pNf);

    DatabaseConnection_service::open();
    QSqlQuery qId(db);
    qId.prepare("SELECT id FROM produtos WHERE codigo_barras = :cod");
    qId.bindValue(":cod", "NF_EMIT_ALL_B");
    qId.exec();
    qId.next();
    qlonglong idProdComNf = qId.value(0).toLongLong();
    db.close();

    QVERIFY(idProdSemNf > 0);
    QVERIFY(idProdComNf > 0);

    VendasDTO v;
    v.clienteNome    = "Consumidor";
    v.dataHora       = "2026-03-16 10:00:00";
    v.total          = 35.00;
    v.formaPagamento = "Dinheiro";
    v.valorRecebido  = 35.00;
    v.troco          = 0;
    v.taxa           = 0;
    v.valorFinal     = 35.00;
    v.desconto       = 0;
    v.estaPago       = true;
    v.idCliente      = 1;

    ProdutoVendidoDTO pv1;
    pv1.idProduto    = idProdSemNf;
    pv1.idVenda      = 0;
    pv1.quantidade   = 1;
    pv1.precoVendido = 15.00;

    ProdutoVendidoDTO pv2;
    pv2.idProduto    = idProdComNf;
    pv2.idVenda      = 0;
    pv2.quantidade   = 1;
    pv2.precoVendido = 20.00;

    Vendas_service vendaService;
    auto ri = vendaService.inserirVendaRegraDeNegocio(v, {pv1, pv2});
    QVERIFY(ri.ok);

    ProdutoVenda_service service;
    auto r = service.marcarComoEmitidoNF(ri.idVendaInserida, true);
    QVERIFY(r.ok);

    // ambos devem estar marcados como emitido_nf = 1
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec(QString("SELECT emitido_nf FROM produtos_vendidos WHERE id_venda = %1 AND id_produto = %2")
               .arg(ri.idVendaInserida).arg(idProdSemNf));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 1);

    QSqlQuery q2(db);
    q2.exec(QString("SELECT emitido_nf FROM produtos_vendidos WHERE id_venda = %1 AND id_produto = %2")
                .arg(ri.idVendaInserida).arg(idProdComNf));
    QVERIFY(q2.next());
    QCOMPARE(q2.value(0).toInt(), 1);
    db.close();
}

void TestProdutoVendaService::marcar_emitido_nf_apenas_produtos_nf()
{
    // produto nf=false
    qlonglong idProdSemNf = inserirProdutoTeste(db, "NF_ONLY_A");
    // produto nf=true
    Produto_Service ps;
    ProdutoDTO pNf;
    pNf.quantidade      = 10;
    pNf.descricao       = "Produto NF True B";
    pNf.preco           = 20.00;
    pNf.codigoBarras    = "NF_ONLY_B";
    pNf.nf              = true;
    pNf.uCom            = "UN";
    pNf.precoFornecedor = 10.00;
    pNf.percentLucro    = 20.00;
    pNf.ncm             = "12345678";
    pNf.cest            = "";
    pNf.aliquotaIcms    = 18;
    pNf.csosn           = "102";
    pNf.pis             = "49";
    ps.inserir(pNf);

    DatabaseConnection_service::open();
    QSqlQuery qId(db);
    qId.prepare("SELECT id FROM produtos WHERE codigo_barras = :cod");
    qId.bindValue(":cod", "NF_ONLY_B");
    qId.exec();
    qId.next();
    qlonglong idProdComNf = qId.value(0).toLongLong();
    db.close();

    QVERIFY(idProdSemNf > 0);
    QVERIFY(idProdComNf > 0);

    VendasDTO v;
    v.clienteNome    = "Consumidor";
    v.dataHora       = "2026-03-16 10:00:00";
    v.total          = 35.00;
    v.formaPagamento = "Dinheiro";
    v.valorRecebido  = 35.00;
    v.troco          = 0;
    v.taxa           = 0;
    v.valorFinal     = 35.00;
    v.desconto       = 0;
    v.estaPago       = true;
    v.idCliente      = 1;

    ProdutoVendidoDTO pv1;
    pv1.idProduto    = idProdSemNf;
    pv1.idVenda      = 0;
    pv1.quantidade   = 1;
    pv1.precoVendido = 15.00;

    ProdutoVendidoDTO pv2;
    pv2.idProduto    = idProdComNf;
    pv2.idVenda      = 0;
    pv2.quantidade   = 1;
    pv2.precoVendido = 20.00;

    Vendas_service vendaService;
    auto ri = vendaService.inserirVendaRegraDeNegocio(v, {pv1, pv2});
    QVERIFY(ri.ok);

    ProdutoVenda_service service;
    auto r = service.marcarComoEmitidoNF(ri.idVendaInserida, false);
    QVERIFY(r.ok);

    // produto sem nf deve permanecer emitido_nf = 0
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec(QString("SELECT emitido_nf FROM produtos_vendidos WHERE id_venda = %1 AND id_produto = %2")
               .arg(ri.idVendaInserida).arg(idProdSemNf));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 0);

    // produto com nf deve estar emitido_nf = 1
    QSqlQuery q2(db);
    q2.exec(QString("SELECT emitido_nf FROM produtos_vendidos WHERE id_venda = %1 AND id_produto = %2")
                .arg(ri.idVendaInserida).arg(idProdComNf));
    QVERIFY(q2.next());
    QCOMPARE(q2.value(0).toInt(), 1);
    db.close();
}

void TestProdutoVendaService::tem_apenas_um_produto_false()
{
    qlonglong idProd1 = inserirProdutoTeste(db, "PV006");
    qlonglong idProd2 = inserirProdutoTeste(db, "PV007");

    VendasDTO v;
    v.clienteNome    = "Consumidor";
    v.dataHora       = "2026-03-16 10:00:00";
    v.total          = 30.00;
    v.formaPagamento = "Dinheiro";
    v.valorRecebido  = 30.00;
    v.troco          = 0;
    v.taxa           = 0;
    v.valorFinal     = 30.00;
    v.desconto       = 0;
    v.estaPago       = true;
    v.idCliente      = 1;

    ProdutoVendidoDTO pv1;
    pv1.idProduto    = idProd1;
    pv1.idVenda      = 0;
    pv1.quantidade   = 1;
    pv1.precoVendido = 15.00;

    ProdutoVendidoDTO pv2;
    pv2.idProduto    = idProd2;
    pv2.idVenda      = 0;
    pv2.quantidade   = 1;
    pv2.precoVendido = 15.00;

    Vendas_service vendaService;
    auto ri = vendaService.inserirVendaRegraDeNegocio(v, {pv1, pv2});
    QVERIFY(ri.ok);

    ProdutoVenda_service service;
    QVERIFY(!service.temApenasUmProduto(ri.idVendaInserida));
}
