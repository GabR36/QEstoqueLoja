#ifndef PRODUTO_H
#define PRODUTO_H

#include <QString>


class Produto
{
public:
    int id;
    QString nome;
    QString descricao;
    int quantidade;
    Produto(QString nomeArg, QString descArg, int quantArg, int idArg);
};

#endif // PRODUTO_H
