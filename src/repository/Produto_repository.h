#ifndef PRODUTO_REPOSITORY_H
#define PRODUTO_REPOSITORY_H

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QVariantMap>
#include "../dto/Produto_dto.h"

class Produto_Repository : public QObject
{
    Q_OBJECT
public:
    explicit Produto_Repository(QObject *parent = nullptr);

    bool codigoBarrasExiste(const QString &codigo);
    bool inserir(const ProdutoDTO &p, QString &erroSQL);
    QSqlQueryModel *getProdutoPeloCodigo(const QString &codigoBarras);
    bool deletar(const QString &id, QString &erroSQL);
    bool alterar(const ProdutoDTO &p, const QString &id, QString &erro);
    QStringList listarLocais();
    bool atualizarLocal(int id, const QString &local, QString &erroSQL);
    bool updateAumentarQuantidadeProduto(qlonglong idprod, double quantia);
    ProdutoDTO getProduto(qlonglong id);
    void listarProdutos(QSqlQueryModel *model);
    void pesquisar(const QStringList &palavras, const QString &textoNormalizado, QSqlQueryModel *model);
    ProdutoDTO getProdutoPeloCodBarras(const QString &codigo);
    bool updateDiminuirQuantidadeProduto(qlonglong idprod, double quantia);
    QVariantMap getProdutoPorCodBarrasMap(const QString &codigo);
    bool atualizarCamposMap(qlonglong id, const QVariantMap &campos, bool marcarNf);
private:
    QSqlDatabase db;
};

#endif // PRODUTO_REPOSITORY_H
