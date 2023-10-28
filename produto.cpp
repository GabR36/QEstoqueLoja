#include "produto.h"


Produto::Produto(QString nomeArg, QString descArg, int quantArg, int idArg)
{
    id = idArg;
    nome = nomeArg;
    descricao = descArg;
    quantidade = quantArg;
}
