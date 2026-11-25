#ifndef TEST_FINANCEIRO_SERVICE_H
#define TEST_FINANCEIRO_SERVICE_H

#include <QObject>
#include <QSqlDatabase>

// Testes do calculo de lucro das vendas (Financeiro_service) e dos
// relatorios de lucro (Relatorios_service / Relatorios_repository).
class TestFinanceiroService : public QObject
{
    Q_OBJECT
private:
    QSqlDatabase db;

private slots:
    void init();
    void cleanup();

    // Financeiro_service::getLucroEstimadoVendaDto
    void lucro_venda_com_preco_fornecedor();
    void lucro_venda_sem_preco_fornecedor_usa_porcentagem();
    void lucro_venda_desconta_desconto_e_taxa_cartao();
    void lucro_venda_taxa_ignorada_fora_do_cartao();
    void lucro_venda_varios_produtos();

    // persistencia do lucro na venda
    void inserir_venda_salva_lucro();
    void devolucao_recalcula_lucro();

    // relatorios
    void relatorio_lucro_periodo_vendas_a_vista();
    void relatorio_lucro_periodo_vendas_a_prazo();
    void relatorio_produtos_mais_lucrativos_rateia_lucro();
};

#endif // TEST_FINANCEIRO_SERVICE_H
