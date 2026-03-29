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
    q.exec("DELETE FROM eventos_fiscais");
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

void TestFiscalEmitterService::enviarInutilizacao_retornoForcado_ok()
{
    QFile file(":/recursos/inu_sanitizada_ok.txt");
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Erro ao abrir inu_sanitizada_ok.txt");
    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();
    QVERIFY(!retornoFake.isEmpty());

    FiscalEmitter_service service;
    service.setRetornoForcado(retornoFake);

    auto r = service.enviarInutilizacao(ModeloNF::NFCe,
                                        "Erro de sequencia numerica no sistema",
                                        588, 588);

    QVERIFY2(r.ok, qPrintable(r.msg));
    QCOMPARE(r.erro, FiscalEmitterErro::Nenhum);

    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec("SELECT tipo_evento, cstat, nprot, codigo FROM eventos_fiscais ORDER BY id DESC LIMIT 1");
    QVERIFY2(q.next(), "Nenhum evento fiscal inserido no banco");
    QCOMPARE(q.value("tipo_evento").toString(), QString("INUTILIZACAO"));
    QCOMPARE(q.value("cstat").toString(),       QString("102"));
    QCOMPARE(q.value("nprot").toString(),       QString("141260000000000"));
    QCOMPARE(q.value("codigo").toString(),      QString("mod65|588-588"));
    db.close();
}

void TestFiscalEmitterService::enviarNfcePadrao_retornoForcado_windows_ok()
{
    // retorno capturado no Windows: path com barras invertidas, nNF=593
    QFile file(":/recursos/nfce_sanitizada_windows.txt");
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Erro ao abrir nfce_sanitizada_windows.txt");
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

    FiscalEmitter_service service;
    service.setRetornoForcado(retornoFake);

    auto r = service.enviarNfcePadrao(venda, {pv}, 593, cliente, true, false);

    QVERIFY2(r.ok, qPrintable(r.msg));

    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec("SELECT cstat, chnfe FROM notas_fiscais ORDER BY id DESC LIMIT 1");
    QVERIFY(q.next());
    QCOMPARE(q.value("cstat").toString(), QString("100"));
    QCOMPARE(q.value("chnfe").toString(),
             QString("41260399999999000191650010000005931000000090"));
    db.close();
}

void TestFiscalEmitterService::enviarInutilizacao_motivo_curto()
{
    FiscalEmitter_service service;
    auto r = service.enviarInutilizacao(ModeloNF::NFCe, "Curto", 1, 1);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, FiscalEmitterErro::QuebraDeRegra);
}

void TestFiscalEmitterService::enviarInutilizacao_numeros_invalidos()
{
    FiscalEmitter_service service;

    auto r1 = service.enviarInutilizacao(ModeloNF::NFCe, "Justificativa valida aqui", 0, 5);
    QVERIFY(!r1.ok);
    QCOMPARE(r1.erro, FiscalEmitterErro::QuebraDeRegra);

    auto r2 = service.enviarInutilizacao(ModeloNF::NFCe, "Justificativa valida aqui", 10, 5);
    QVERIFY(!r2.ok);
    QCOMPARE(r2.erro, FiscalEmitterErro::QuebraDeRegra);
}

void TestFiscalEmitterService::enviarInutilizacao_retornoForcado_recusado()
{
    QFile file(":/recursos/inu_sanitizada_recusado.txt");
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Erro ao abrir inu_sanitizada_recusado.txt");
    QString retornoFake = QString::fromUtf8(file.readAll());
    file.close();
    QVERIFY(!retornoFake.isEmpty());

    FiscalEmitter_service service;
    service.setRetornoForcado(retornoFake);

    auto r = service.enviarInutilizacao(ModeloNF::NFCe,
                                        "Erro de sequencia numerica no sistema",
                                        588, 588);

    QVERIFY(!r.ok);
    QCOMPARE(r.erro, FiscalEmitterErro::Recusado);
    QVERIFY2(r.msg.contains("563"), qPrintable("Esperado CStat 563 na mensagem: " + r.msg));

    // nenhum evento deve ter sido salvo no banco
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.exec("SELECT COUNT(*) FROM eventos_fiscais");
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 0);
    db.close();
}
