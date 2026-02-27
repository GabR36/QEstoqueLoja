#ifndef PRODUTO_REPOSITORY_H
#define PRODUTO_REPOSITORY_H

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "../dto/Produto_dto.h"

class Produto_Repository : public QObject
{
    Q_OBJECT
public:
    explicit Produto_Repository(QObject *parent = nullptr);

    bool codigoBarrasExiste(const QString &codigo);
    bool inserir(const ProdutoDTO &p, QString &erroSQL);
    QSqlQueryModel* listarProdutos();
    QSqlQueryModel *getProdutoPeloCodigo(const QString &codigoBarras);
    bool deletar(const QString &id, QString &erroSQL);
    QSqlQueryModel *pesquisar(const QStringList &palavras, const QString &textoNormalizado);
    bool alterar(const ProdutoDTO &p, const QString &id, QString &erro);
    QStringList listarLocais();
    bool atualizarLocal(int id, const QString &local, QString &erroSQL);
    bool updateAumentarQuantidadeProduto(qlonglong idprod, double quantia);
private:
    QSqlDatabase db;
};

#endif // PRODUTO_REPOSITORY_H
