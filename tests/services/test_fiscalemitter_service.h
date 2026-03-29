#ifndef TEST_FISCALEMITTER_SERVICE_H
#define TEST_FISCALEMITTER_SERVICE_H

#include <QObject>
#include <QSqlDatabase>

class TestFiscalEmitterService : public QObject
{
    Q_OBJECT
private:
    QSqlDatabase db;

private slots:
    void init();
    void cleanup();
    void enviarNfcePadrao_retornoForcado_ok();
    void enviarNfcePadrao_cpf_invalido();
    void enviarNfcePadrao_sem_produtos_nf();
    void enviarNFePadrao_retornoForcado_ok();
    void enviarNFePadrao_cliente_incompleto();
    void enviarInutilizacao_retornoForcado_ok();
    void enviarInutilizacao_motivo_curto();
    void enviarInutilizacao_numeros_invalidos();
    void enviarInutilizacao_retornoForcado_recusado();
    void enviarNfcePadrao_retornoForcado_windows_ok();
};

#endif // TEST_FISCALEMITTER_SERVICE_H
