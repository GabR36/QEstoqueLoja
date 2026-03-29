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
    struct ResultadoValidacaoClienteCompleter {
        bool ok = false;
        ClienteErro erro = ClienteErro::Nenhum;
        QString msg;
        qlonglong clienteId = -1;
        QString nomeCorrigido;
    };
    explicit Cliente_service(QObject *parent = nullptr);
    qlonglong contarQuantosRegistrosPorCPFCNPJ(const QString &cpfcnpj);
    Cliente_service::Resultado inserirClienteEmitente(ClienteDTO emissor);
    qlonglong getIdFromCpfCnpj(const QString &cpfcnpj);
    void listarClientes(QSqlQueryModel *model);
    Cliente_service::Resultado deletarCliente(qlonglong id);
    void pesquisar(QSqlQueryModel* model, const QString &nome);
    Cliente_service::Resultado inserirCliente(ClienteDTO cliente);
    ClienteDTO getClienteByID(qlonglong id);
    Cliente_service::Resultado updateCliente(qlonglong id, ClienteDTO cliente);
    QStringList listarClientesParaCompleter();
    Cliente_service::ResultadoValidacaoClienteCompleter validarClienteTexto(const QString &texto);
    QPair<QString, int> extrairNomeId(const QString &texto);
private:
    Cliente_repository cliRepo;

signals:
};

#endif // CLIENTE_SERVICE_H
