#ifndef CLIENTE_SERVICE_H
#define CLIENTE_SERVICE_H

#include <QObject>
#include "../repository/cliente_repository.h"
#include <QSqlQueryModel>

enum class ClienteErro {
    Nenhum,
    CampoVazio,
    Banco,
    InsercaoInvalida,
    DeleteFalhou,
    QuebraDeRegra
};

class Cliente_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        ClienteErro erro = ClienteErro::Nenhum;
        QString msg;
    };
    explicit Cliente_service(QObject *parent = nullptr);
    qlonglong contarQuantosRegistrosPorCPFCNPJ(const QString &cpfcnpj);
    Cliente_service::Resultado inserirClienteEmitente(ClienteDTO emissor);
    qlonglong getIdFromCpfCnpj(const QString &cpfcnpj);
    QSqlQueryModel *listarClientes();
    Cliente_service::Resultado deletarCliente(qlonglong id);
    QSqlQueryModel *pesquisar(const QString &nome);
private:
    Cliente_repository cliRepo;

signals:
};

#endif // CLIENTE_SERVICE_H
