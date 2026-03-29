#ifndef TEST_VENDAS_SERVICE_H
#define TEST_VENDAS_SERVICE_H

#include <QObject>
#include <QSqlDatabase>

class TestVendasService : public QObject
{
    Q_OBJECT
private:
    QSqlDatabase db;

private slots:
    void init();
    void cleanup();
    void inserir_venda_ok();
    void erro_desconto_maior_que_total();
    void erro_prazo_sem_cliente();
    void deletar_venda_ok();
    void devolver_produto_ok();
    void calcular_resumo();
};

#endif // TEST_VENDAS_SERVICE_H
