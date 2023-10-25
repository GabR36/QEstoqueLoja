#ifndef PRODUTO_H
#define PRODUTO_H

#include <QString>


class Produto
{
public:
    QString nome;
    QString descricao;
    int quantidade;
    Produto(QString nomeArg, QString descArg, int quantArg);
};

#endif // PRODUTO_H
