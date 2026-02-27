#ifndef TEST_PRODUTO_SERVICE_H
#define TEST_PRODUTO_SERVICE_H

#include <QObject>
#include <QSqlDatabase>


class TestProdutoService : public QObject
{
    Q_OBJECT
private:
        QSqlDatabase db;

private slots:
    void inserir_produto_ok();
    void erro_codigo_barras_existente();
    void erro_preco_invalido();
    void erro_quantidade_invalida();
    void erro_ncm_nf();
    void cleanup();
    void inserir_produto_errado();
    void deletar_produto_ok();
    void deletar_produto_inexistente();
    void alterar_produto_ok();
    void init();
};

#endif // TEST_PRODUTO_SERVICE_H
