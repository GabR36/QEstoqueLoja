#ifndef CLIENTE_REPOSITORY_H
#define CLIENTE_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include "../dto/Cliente_dto.h"
#include <QSqlQueryModel>

class Cliente_repository : public QObject
{
    Q_OBJECT
public:
    explicit Cliente_repository(QObject *parent = nullptr);
    qlonglong contarQuantosRegistrosPorCPFCNPJ(const QString &cpfcnpj);
    bool inserir(ClienteDTO cliente);
    qlonglong getIdFromCPFCNPJ(const QString &cpfcnpj);
    QSqlQueryModel *listarClientes();
    bool deletarCliente(qlonglong id);
    QSqlQueryModel *pesquisar(const QString &nome);
    ClienteDTO getClienteByID(qlonglong id);
    bool updateCliente(qlonglong id, ClienteDTO cliente);
private:
    QSqlDatabase db;

signals:
};

#endif // CLIENTE_REPOSITORY_H
