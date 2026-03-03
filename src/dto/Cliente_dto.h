#ifndef CLIENTE_DTO_H
#define CLIENTE_DTO_H
#include <QString>

struct ClienteDTO {
    QString nome;
    QString email;
    QString telefone;
    QString endereco;
    QString cpf;
    QString dataNasc;
    QString dataCadastro;
    bool ehPf;
    qlonglong numeroEnd = 0;
    QString bairro;
    QString xMun;
    QString cMun;
    QString uf;
    QString cep;
    int indIeDest;
    QString ie;

};

#endif // CLIENTE_DTO_H
