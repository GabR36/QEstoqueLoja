#ifndef TEST_CLIENTE_SERVICE_H
#define TEST_CLIENTE_SERVICE_H

#include <QObject>
#include <QSqlDatabase>


class test_cliente_service : public QObject
{
    Q_OBJECT
public:
    explicit test_cliente_service(QObject *parent = nullptr);
private:
    QSqlDatabase db;

signals:
private slots:
    void init();
    void cleanup();
    void inserir_cliente_nome_vazio();
    void erro_cpf_duplicado();
    void erro_email_invalido();
    void erro_telefone_invalido();
    void deletar_cliente_ok();
    void deletar_cliente_protegido();
    void alterar_cliente_ok();
    void alterar_cliente_invalido();
};

#endif // TEST_CLIENTE_SERVICE_H
