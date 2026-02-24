#ifndef PRODUTOVENDA_REPOSITORY_H
#define PRODUTOVENDA_REPOSITORY_H

#include <QObject>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include "../dto/ProdutoVendido_dto.h"
class ProdutoVenda_repository : public QObject
{
    Q_OBJECT
public:
    explicit ProdutoVenda_repository(QObject *parent = nullptr);
    QSqlQueryModel *listarProdutosVenda();
    QList<ProdutoVendidoDTO> getProdutosVendidos(qlonglong idVenda);
    QSqlQueryModel *listarProdutosVendidosFromVenda(qlonglong idvenda);
private:
    QSqlDatabase db;

signals:
};

#endif // PRODUTOVENDA_REPOSITORY_H
