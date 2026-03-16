#include <QtTest>
#include <QFile>
#include <QSqlQuery>
#include "test_fiscalemitter_service.h"
#include "services/fiscalemitter_service.h"
#include "services/Produto_service.h"
#include "../db/test_db_factory.h"
#include "infra/databaseconnection_service.h"

namespace {

qlonglong inserirProdutoNf(QSqlDatabase &db)
{
    Produto_Service ps;
    ProdutoDTO p;
    p.quantidade      = 10;
    p.descricao       = "Produto NF Teste";
    p.preco           = 50.00;
    p.codigoBarras    = "FE001";
    p.nf              = true;
    p.uCom            = "UN";
    p.precoFornecedor = 30.00;
    p.percentLucro    = 20.00;
    p.ncm             = "22021000"; // NCM válido
    p.cest            = "";
    p.aliquotaIcms    = 18;
    p.csosn           = "102";
    p.pis             = "49";
    ps.inserir(p);

    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.prepare("SELECT id FROM produtos WHERE codigo_barras = 'FE001'");
    q.exec();
    q.next();
    qlonglong id = q.value(0).toLongLong();
    db.close();
    return id;
}

} // namespace

void TestFiscalEmitterService::init()
{
    db = DatabaseConnection_service::db();
}

void TestFiscalEmitterService::cleanup()
{
    if (!DatabaseConnection_service::open())
        return;
    QSqlQuery q(db);
    q.exec("DELETE FROM notas_fiscais");
    q.exec("DELETE FROM produtos");
    db.close();
}

void TestFiscalEmitterService::enviarNfcePadrao_retornoForcado_ok()
{
    // carrega retorno ACBr fake do arquivo de recurso
    QFile file(":/recursos/nfce_sanitizada_ok.txt");
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Erro ao abrir nfce_sanitizada_ok.txt");
    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();
    QVERIFY(!retornoFake.isEmpty());

    qlonglong idProd = inserirProdutoNf(db);
    QVERIFY(idProd > 0);

    VendasDTO venda;
    venda.id             = 1;
    venda.total          = 50.00;
    venda.formaPagamento = "Dinheiro";
    venda.valorRecebido  = 50.00;
    venda.troco          = 0;
    venda.taxa           = 0;
    venda.desconto       = 0;

    ProdutoVendidoDTO pv;
    pv.idProduto    = idProd;
    pv.idVenda      = 1;
    pv.quantidade   = 1;
    pv.precoVendido = 50.00;

    ClienteDTO cliente;
    // sem CPF: consumidor final

    FiscalEmitter_service service;
    service.setRetornoForcado(retornoFake);

    // emitirTodos=true para incluir todos os produtos; ignorarNCM=false
    auto r = service.enviarNfcePadrao(venda, {pv}, 582, cliente, true, false);

    QVERIFY2(r.ok, qPrintable(r.msg));

    // verifica que a nota foi salva no banco com cstat=100
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec("SELECT cstat, chnfe FROM notas_fiscais ORDER BY id DESC LIMIT 1");
    QVERIFY(q.next());
    QCOMPARE(q.value("cstat").toString(), QString("100"));
    QCOMPARE(q.value("chnfe").toString(),
             QString("41260399999999000191650010000005821000000090"));
    db.close();
}

void TestFiscalEmitterService::enviarNfcePadrao_cpf_invalido()
{
    VendasDTO venda;
    venda.id = 1;
    venda.total = 50.00;
    venda.formaPagamento = "Dinheiro";
    venda.valorRecebido  = 50.00;

    ClienteDTO cliente;
    cliente.cpf  = "12345"; // CPF com tamanho errado
    cliente.ehPf = true;

    FiscalEmitter_service service;
    auto r = service.enviarNfcePadrao(venda, {}, 1, cliente, false, false);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, FiscalEmitterErro::QuebraDeRegra);
}

void TestFiscalEmitterService::enviarNfcePadrao_sem_produtos_nf()
{
    qlonglong idProd = inserirProdutoNf(db);
    QVERIFY(idProd > 0);

    // produto sem nf=true no banco: vamos inserir um produto sem NF
    Produto_Service ps;
    ProdutoDTO p;
    p.quantidade      = 5;
    p.descricao       = "Produto Sem NF";
    p.preco           = 10.00;
    p.codigoBarras    = "FE002";
    p.nf              = false;
    p.uCom            = "UN";
    p.precoFornecedor = 5.00;
    p.percentLucro    = 10.00;
    p.ncm             = "";
    p.csosn           = "102";
    p.pis             = "49";
    ps.inserir(p);

    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec("SELECT id FROM produtos WHERE codigo_barras = 'FE002'");
    q.next();
    qlonglong idProdSemNf = q.value(0).toLongLong();
    db.close();

    VendasDTO venda;
    venda.id = 1;
    venda.total = 10.00;
    venda.formaPagamento = "Dinheiro";
    venda.valorRecebido  = 10.00;

    ProdutoVendidoDTO pv;
    pv.idProduto    = idProdSemNf;
    pv.quantidade   = 1;
    pv.precoVendido = 10.00;

    ClienteDTO cliente;

    FiscalEmitter_service service;
    // emitirTodos=false → só inclui produtos com nf=true → nenhum → ProdutosSemNF
    auto r = service.enviarNfcePadrao(venda, {pv}, 1, cliente, false, false);

    // Retorna ok=true mas com erro ProdutosSemNF (nota não enviada, mas não é falha)
    QVERIFY(r.ok);
    QCOMPARE(r.erro, FiscalEmitterErro::ProdutosSemNF);
}

void TestFiscalEmitterService::enviarNFePadrao_retornoForcado_ok()
{
    QFile file(":/recursos/nfe_sanitizada_ok.txt");
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Erro ao abrir nfe_sanitizada_ok.txt");
    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();
    QVERIFY(!retornoFake.isEmpty());

    qlonglong idProd = inserirProdutoNf(db);
    QVERIFY(idProd > 0);

    VendasDTO venda;
    venda.id             = 1;
    venda.total          = 50.00;
    venda.formaPagamento = "Dinheiro";
    venda.valorRecebido  = 50.00;
    venda.troco          = 0;
    venda.taxa           = 0;
    venda.desconto       = 0;

    ProdutoVendidoDTO pv;
    pv.idProduto    = idProd;
    pv.idVenda      = 1;
    pv.quantidade   = 1;
    pv.precoVendido = 50.00;

    // NFe exige cliente com todos os campos preenchidos
    ClienteDTO cliente;
    cliente.nome      = "Cliente Teste";
    cliente.cpf       = "99999999000191"; // CNPJ fake (14 dígitos, ehPf=false)
    cliente.ehPf      = false;
    cliente.email     = "teste@example.com";
    cliente.endereco  = "Rua Teste";
    cliente.numeroEnd = 100;
    cliente.bairro    = "Centro";
    cliente.xMun      = "Curitiba";
    cliente.cMun      = "4106902";
    cliente.uf        = "PR";
    cliente.cep       = "80000000";

    FiscalEmitter_service service;
    service.setRetornoForcado(retornoFake);

    auto r = service.enviarNFePadrao(venda, {pv}, 366, cliente, true, false);

    QVERIFY2(r.ok, qPrintable(r.msg));

    // verifica que a nota foi salva com cstat=100 e a chave correta
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec("SELECT cstat, chnfe FROM notas_fiscais ORDER BY id DESC LIMIT 1");
    QVERIFY(q.next());
    QCOMPARE(q.value("cstat").toString(), QString("100"));
    QCOMPARE(q.value("chnfe").toString(),
             QString("41260399999999000191550010000003661000000090"));
    db.close();
}

void TestFiscalEmitterService::enviarNFePadrao_cliente_incompleto()
{
    VendasDTO venda;
    venda.id = 1;
    venda.total = 50.00;
    venda.formaPagamento = "Dinheiro";
    venda.valorRecebido  = 50.00;

    // cliente sem campos obrigatórios
    ClienteDTO cliente;
    cliente.nome  = "Cliente Incompleto";
    cliente.cpf   = "99999999000191";
    cliente.ehPf  = false;
    // email, endereco, etc. vazios

    FiscalEmitter_service service;
    auto r = service.enviarNFePadrao(venda, {}, 1, cliente, false, false);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, FiscalEmitterErro::QuebraDeRegra);
}
