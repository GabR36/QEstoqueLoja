#ifndef TEST_PRODUTOVENDA_SERVICE_H
#define TEST_PRODUTOVENDA_SERVICE_H

#include <QObject>
#include <QSqlDatabase>

class TestProdutoVendaService : public QObject
{
    Q_OBJECT
private:
    QSqlDatabase db;

private slots:
    void init();
    void cleanup();
    void inserir_produto_vendido_ok();
    void deletar_produto_vendido_ok();
    void deletar_por_idvenda();
    void tem_apenas_um_produto_true();
    void tem_apenas_um_produto_false();
    void marcar_emitido_nf_emitir_todos();
    void marcar_emitido_nf_apenas_produtos_nf();
};

#endif // TEST_PRODUTOVENDA_SERVICE_H
