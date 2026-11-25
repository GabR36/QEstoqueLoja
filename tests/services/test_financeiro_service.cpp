#include <QtTest>
#include "test_financeiro_service.h"
#include "services/financeiro_service.h"
#include "repository/relatorios_repository.h"
#include "services/vendas_service.h"
#include "services/Produto_service.h"
#include "infra/databaseconnection_service.h"
#include <QSqlQuery>
#include <QSqlError>

namespace {

constexpr double EPS = 0.001;

#define COMPARAR_DINHEIRO(atual, esperado) \
    QVERIFY2(qAbs((atual) - (esperado)) < EPS, \
             qPrintable(QString("atual=%1 esperado=%2").arg(atual, 0, 'f', 4).arg(esperado, 0, 'f', 4)))

qlonglong inserirProduto(QSqlDatabase &db, const QString &codBarras, const QString &descricao,
                         double preco, double precoFornecedor, double percentLucro)
{
    Produto_Service ps;
    ProdutoDTO p;
    p.quantidade      = 100;
    p.descricao       = descricao;
    p.preco           = preco;
    p.codigoBarras    = codBarras;
    p.nf              = false;
    p.uCom            = "UN";
    p.precoFornecedor = precoFornecedor;
    p.percentLucro    = percentLucro;
    p.ncm             = "";
    p.cest            = "";
    p.aliquotaIcms    = 18;
    p.csosn           = "102";
    p.pis             = "49";
    auto r = ps.inserir(p);
    if (!r.ok) {
        qWarning() << "Falha inserir produto teste financeiro:" << r.msg;
        return -1;
    }

    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.prepare("SELECT id FROM produtos WHERE codigo_barras = :cod");
    q.bindValue(":cod", codBarras);
    q.exec();
    q.next();
    return q.value(0).toLongLong();
}

ProdutoVendidoDTO item(qlonglong idProd, double quantidade, double precoVendido)
{
    ProdutoVendidoDTO pv;
    pv.id           = 0;
    pv.idProduto    = idProd;
    pv.idVenda      = 0;
    pv.quantidade   = quantidade;
    pv.precoVendido = precoVendido;
    return pv;
}

VendasDTO venda(double total, const QString &formaPag = "Dinheiro",
                double desconto = 0, double taxa = 0)
{
    VendasDTO v;
    v.clienteNome    = "Consumidor";
    v.dataHora       = "2026-03-16 10:00:00";
    v.total          = total;
    v.formaPagamento = formaPag;
    v.desconto       = desconto;
    v.taxa           = taxa;
    v.valorFinal     = (total - desconto) * (1 + taxa / 100.0);
    v.valorRecebido  = v.valorFinal;
    v.troco          = 0;
    v.estaPago       = true;
    v.idCliente      = 1;
    return v;
}

// insere direto na tabela, para controlar data, forma de pagamento e lucro
// gravado sem depender das regras de negocio da venda.
qlonglong inserirVendaSql(QSqlDatabase &db, const QString &dataHora, const QString &formaPag,
                          double total, double valorFinal, double lucro,
                          double desconto = 0, double taxa = 0)
{
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.prepare("INSERT INTO vendas2 (cliente, total, data_hora, forma_pagamento, valor_recebido, "
              "troco, taxa, valor_final, desconto, id_cliente, esta_pago, lucro) "
              "VALUES ('Consumidor', :total, :dh, :fp, :vf, 0, :taxa, :vf2, :desc, 1, :pago, :lucro)");
    q.bindValue(":total", total);
    q.bindValue(":dh", dataHora);
    q.bindValue(":fp", formaPag);
    q.bindValue(":vf", valorFinal);
    q.bindValue(":taxa", taxa);
    q.bindValue(":vf2", valorFinal);
    q.bindValue(":desc", desconto);
    q.bindValue(":pago", formaPag != "Prazo");
    q.bindValue(":lucro", lucro);
    if (!q.exec()) {
        qWarning() << "Falha inserirVendaSql:" << q.lastError().text();
        return -1;
    }
    return q.lastInsertId().toLongLong();
}

bool inserirProdutoVendidoSql(QSqlDatabase &db, qlonglong idVenda, qlonglong idProd,
                              double quantidade, double precoVendido)
{
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.prepare("INSERT INTO produtos_vendidos (id_produto, id_venda, quantidade, preco_vendido) "
              "VALUES (:p, :v, :q, :pr)");
    q.bindValue(":p", idProd);
    q.bindValue(":v", idVenda);
    q.bindValue(":q", quantidade);
    q.bindValue(":pr", precoVendido);
    if (!q.exec()) {
        qWarning() << "Falha inserirProdutoVendidoSql:" << q.lastError().text();
        return false;
    }
    return true;
}

bool inserirEntradaSql(QSqlDatabase &db, qlonglong idVenda, const QString &dataHora,
                       const QString &formaPag, double total, double valorFinal, double taxa = 0)
{
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.prepare("INSERT INTO entradas_vendas (id_venda, total, data_hora, forma_pagamento, "
              "valor_recebido, troco, taxa, valor_final, desconto) "
              "VALUES (:v, :t, :dh, :fp, :vf, 0, :taxa, :vf2, 0)");
    q.bindValue(":v", idVenda);
    q.bindValue(":t", total);
    q.bindValue(":dh", dataHora);
    q.bindValue(":fp", formaPag);
    q.bindValue(":vf", valorFinal);
    q.bindValue(":taxa", taxa);
    q.bindValue(":vf2", valorFinal);
    if (!q.exec()) {
        qWarning() << "Falha inserirEntradaSql:" << q.lastError().text();
        return false;
    }
    return true;
}

double lucroGravado(QSqlDatabase &db, qlonglong idVenda)
{
    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.prepare("SELECT lucro FROM vendas2 WHERE id = :id");
    q.bindValue(":id", idVenda);
    if (!q.exec() || !q.next())
        return -9999;
    return q.value(0).toDouble();
}

} // namespace

void TestFinanceiroService::init()
{
    db = DatabaseConnection_service::db();
}

void TestFinanceiroService::cleanup()
{
    if (!DatabaseConnection_service::open())
        return;
    QSqlQuery q(db);
    q.exec("DELETE FROM entradas_vendas");
    q.exec("DELETE FROM produtos_vendidos");
    q.exec("DELETE FROM vendas2");
    q.exec("DELETE FROM produtos");
}

// ---------------------------------------------------------------------------
// getLucroEstimadoVendaDto
// ---------------------------------------------------------------------------

void TestFinanceiroService::lucro_venda_com_preco_fornecedor()
{
    // custo 10, vendido 2 x 20 -> lucro 2 * (20 - 10) = 20
    qlonglong idProd = inserirProduto(db, "FIN001", "Prod Fin 1", 20.0, 10.0, 100.0);
    QVERIFY(idProd > 0);

    Financeiro_service fin;
    double lucro = fin.getLucroEstimadoVendaDto(venda(40.0), {item(idProd, 2, 20.0)});

    COMPARAR_DINHEIRO(lucro, 20.0);
}

void TestFinanceiroService::lucro_venda_sem_preco_fornecedor_usa_porcentagem()
{
    // sem fornecedor: custo = preco / (1 + porcent/100) = 24 / 1.2 = 20
    // vendido 3 x 24 -> lucro 3 * (24 - 20) = 12
    qlonglong idProd = inserirProduto(db, "FIN002", "Prod Fin 2", 24.0, 0.0, 20.0);
    QVERIFY(idProd > 0);

    Financeiro_service fin;
    double lucro = fin.getLucroEstimadoVendaDto(venda(72.0), {item(idProd, 3, 24.0)});

    COMPARAR_DINHEIRO(lucro, 12.0);
}

void TestFinanceiroService::lucro_venda_desconta_desconto_e_taxa_cartao()
{
    // lucro itens = 2 * (20 - 10) = 20
    // desconto 5 -> 15
    // credito taxa 4% sobre (40 - 5) = 1.40 -> 13.60
    qlonglong idProd = inserirProduto(db, "FIN003", "Prod Fin 3", 20.0, 10.0, 100.0);
    QVERIFY(idProd > 0);

    Financeiro_service fin;
    double lucroCredito = fin.getLucroEstimadoVendaDto(
        venda(40.0, "Crédito", 5.0, 4.0), {item(idProd, 2, 20.0)});
    COMPARAR_DINHEIRO(lucroCredito, 13.60);

    double lucroDebito = fin.getLucroEstimadoVendaDto(
        venda(40.0, "Débito", 5.0, 2.0), {item(idProd, 2, 20.0)});
    // taxa 2% sobre 35 = 0.70 -> 14.30
    COMPARAR_DINHEIRO(lucroDebito, 14.30);
}

void TestFinanceiroService::lucro_venda_taxa_ignorada_fora_do_cartao()
{
    // forma de pagamento que nao e cartao: taxa nao e descontada
    qlonglong idProd = inserirProduto(db, "FIN004", "Prod Fin 4", 20.0, 10.0, 100.0);
    QVERIFY(idProd > 0);

    Financeiro_service fin;
    double lucro = fin.getLucroEstimadoVendaDto(
        venda(40.0, "Dinheiro", 5.0, 4.0), {item(idProd, 2, 20.0)});

    COMPARAR_DINHEIRO(lucro, 15.0);
}

void TestFinanceiroService::lucro_venda_varios_produtos()
{
    // A: fornecedor 10, 2 x 20          -> 20
    // B: sem fornecedor, 30 com 50%     -> custo 20, 1 x 25 (vendido com desconto no item) -> 5
    // C: fornecedor 8, 0.5 x 12 (granel) -> 2
    qlonglong a = inserirProduto(db, "FIN005", "Prod Fin A", 20.0, 10.0, 100.0);
    qlonglong b = inserirProduto(db, "FIN006", "Prod Fin B", 30.0, 0.0, 50.0);
    qlonglong c = inserirProduto(db, "FIN007", "Prod Fin C", 12.0, 8.0, 50.0);
    QVERIFY(a > 0 && b > 0 && c > 0);

    Financeiro_service fin;
    double lucro = fin.getLucroEstimadoVendaDto(
        venda(71.0),
        {item(a, 2, 20.0), item(b, 1, 25.0), item(c, 0.5, 12.0)});

    COMPARAR_DINHEIRO(lucro, 27.0);
}

// ---------------------------------------------------------------------------
// persistencia
// ---------------------------------------------------------------------------

void TestFinanceiroService::inserir_venda_salva_lucro()
{
    qlonglong idProd = inserirProduto(db, "FIN010", "Prod Fin 10", 20.0, 10.0, 100.0);
    QVERIFY(idProd > 0);

    // mesmo fluxo da tela de venda: calcula o lucro e grava junto com a venda
    VendasDTO v = venda(40.0, "Crédito", 5.0, 4.0);
    QList<ProdutoVendidoDTO> itens = {item(idProd, 2, 20.0)};

    Financeiro_service fin;
    v.lucro = fin.getLucroEstimadoVendaDto(v, itens);

    Vendas_service vendasServ;
    auto r = vendasServ.inserirVendaRegraDeNegocio(v, itens);
    QVERIFY2(r.ok, qPrintable(r.msg));

    COMPARAR_DINHEIRO(lucroGravado(db, r.idVendaInserida), 13.60);
}

void TestFinanceiroService::devolucao_recalcula_lucro()
{
    // A: 2 x 20, custo 10 -> 20
    // B: 1 x 15, custo 5  -> 10
    // desconto 2 -> lucro inicial 28; devolvendo B -> 20 - 2 = 18
    qlonglong a = inserirProduto(db, "FIN011", "Prod Fin 11", 20.0, 10.0, 100.0);
    qlonglong b = inserirProduto(db, "FIN012", "Prod Fin 12", 15.0, 5.0, 200.0);
    QVERIFY(a > 0 && b > 0);

    VendasDTO v = venda(55.0, "Dinheiro", 2.0, 0.0);
    QList<ProdutoVendidoDTO> itens = {item(a, 2, 20.0), item(b, 1, 15.0)};

    Financeiro_service fin;
    v.lucro = fin.getLucroEstimadoVendaDto(v, itens);
    COMPARAR_DINHEIRO(v.lucro, 28.0);

    Vendas_service vendasServ;
    auto r = vendasServ.inserirVendaRegraDeNegocio(v, itens);
    QVERIFY2(r.ok, qPrintable(r.msg));
    COMPARAR_DINHEIRO(lucroGravado(db, r.idVendaInserida), 28.0);

    DatabaseConnection_service::open();
    QSqlQuery q(db);
    q.prepare("SELECT id FROM produtos_vendidos WHERE id_venda = :v AND id_produto = :p");
    q.bindValue(":v", r.idVendaInserida);
    q.bindValue(":p", b);
    QVERIFY(q.exec());
    QVERIFY(q.next());
    qlonglong idProdVendB = q.value(0).toLongLong();

    auto rd = vendasServ.devolverProdutoRegraNegocio(idProdVendB, r.idVendaInserida);
    QVERIFY2(rd.ok, qPrintable(rd.msg));

    COMPARAR_DINHEIRO(lucroGravado(db, r.idVendaInserida), 18.0);
}

// ---------------------------------------------------------------------------
// relatorios
// ---------------------------------------------------------------------------

void TestFinanceiroService::relatorio_lucro_periodo_vendas_a_vista()
{
    // datas em 2030 para nao misturar com outras vendas
    QVERIFY(inserirVendaSql(db, "2030-01-10 09:00:00", "Dinheiro", 40, 40, 20.0) > 0);
    QVERIFY(inserirVendaSql(db, "2030-01-10 15:30:00", "Crédito", 40, 36.4, 13.6, 5, 4) > 0);
    QVERIFY(inserirVendaSql(db, "2030-01-11 11:00:00", "Pix", 30, 30, 15.0) > 0);
    // fora do periodo
    QVERIFY(inserirVendaSql(db, "2030-02-01 10:00:00", "Dinheiro", 100, 100, 50.0) > 0);

    // Relatorios_service::buscarLucroPeriodo delega para Financeiro_service
    Financeiro_service fin;

    auto porDia = fin.getLucroEstimadoPeriodo(QDate(2030, 1, 1), QDate(2030, 1, 31), Agrupamento::Dia);
    QCOMPARE(porDia.size(), 2);
    QVERIFY(porDia.contains("2030-01-10"));
    QVERIFY(porDia.contains("2030-01-11"));
    COMPARAR_DINHEIRO(porDia.value("2030-01-10"), 33.6);
    COMPARAR_DINHEIRO(porDia.value("2030-01-11"), 15.0);

    auto porMes = fin.getLucroEstimadoPeriodo(QDate(2030, 1, 1), QDate(2030, 2, 28), Agrupamento::Mes);
    QCOMPARE(porMes.size(), 2);
    COMPARAR_DINHEIRO(porMes.value("2030-01"), 48.6);
    COMPARAR_DINHEIRO(porMes.value("2030-02"), 50.0);
}

void TestFinanceiroService::relatorio_lucro_periodo_vendas_a_prazo()
{
    // venda a prazo de 100 com lucro 40: o lucro so entra no relatorio
    // conforme as entradas sao pagas, rateado por valor_final da entrada.
    qlonglong idVenda = inserirVendaSql(db, "2030-03-01 10:00:00", "Prazo", 100, 100, 40.0);
    QVERIFY(idVenda > 0);

    QVERIFY(inserirEntradaSql(db, idVenda, "2030-03-05 10:00:00", "Dinheiro", 50, 50));  // 40 * 50/100 = 20
    QVERIFY(inserirEntradaSql(db, idVenda, "2030-04-05 10:00:00", "Pix", 30, 30));       // 40 * 30/100 = 12
    // resto (20) ainda nao pago -> nao entra no lucro

    Financeiro_service fin;

    auto porMes = fin.getLucroEstimadoPeriodo(QDate(2030, 3, 1), QDate(2030, 4, 30), Agrupamento::Mes);
    QCOMPARE(porMes.size(), 2);
    COMPARAR_DINHEIRO(porMes.value("2030-03"), 20.0);
    COMPARAR_DINHEIRO(porMes.value("2030-04"), 12.0);

    // a venda a prazo em si (data 2030-03-01) nao entra como venda a vista
    auto soDia1 = fin.getLucroEstimadoPeriodo(QDate(2030, 3, 1), QDate(2030, 3, 1), Agrupamento::Dia);
    QVERIFY(soDia1.isEmpty());
}

void TestFinanceiroService::relatorio_produtos_mais_lucrativos_rateia_lucro()
{
    // venda com A (2 x 20 = 40) e B (1 x 15 = 15), total itens 55, lucro gravado 22
    // rateio proporcional ao valor: A = 22 * 40/55 = 16, B = 22 * 15/55 = 6
    qlonglong a = inserirProduto(db, "FIN020", "Prod Lucrativo A", 20.0, 10.0, 100.0);
    qlonglong b = inserirProduto(db, "FIN021", "Prod Lucrativo B", 15.0, 5.0, 200.0);
    QVERIFY(a > 0 && b > 0);

    qlonglong v1 = inserirVendaSql(db, "2030-05-10 10:00:00", "Dinheiro", 55, 49, 22.0, 6);
    QVERIFY(v1 > 0);
    QVERIFY(inserirProdutoVendidoSql(db, v1, a, 2, 20.0));
    QVERIFY(inserirProdutoVendidoSql(db, v1, b, 1, 15.0));

    // segunda venda so com A: lucro 10 -> A acumula 26
    qlonglong v2 = inserirVendaSql(db, "2030-05-11 10:00:00", "Dinheiro", 20, 20, 10.0);
    QVERIFY(v2 > 0);
    QVERIFY(inserirProdutoVendidoSql(db, v2, a, 1, 20.0));

    // fora do periodo
    qlonglong v3 = inserirVendaSql(db, "2030-06-01 10:00:00", "Dinheiro", 15, 15, 10.0);
    QVERIFY(v3 > 0);
    QVERIFY(inserirProdutoVendidoSql(db, v3, b, 1, 15.0));

    // Relatorios_service::produtosMaisLucrativosPeriodo delega para o repository
    Relatorios_repository rel;
    auto dados = rel.produtosMaisLucrativosPeriodo(QDate(2030, 5, 1), QDate(2030, 5, 31));

    QCOMPARE(dados.size(), 2);
    COMPARAR_DINHEIRO(double(dados.value("Prod Lucrativo A")), 26.0);
    COMPARAR_DINHEIRO(double(dados.value("Prod Lucrativo B")), 6.0);
}
