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
    void listarProdutosVenda(QSqlQueryModel *model);
    QList<ProdutoVendidoDTO> getProdutosVendidos(qlonglong idVenda);
    void listarProdutosVendidosFromVenda(qlonglong idvenda, QSqlQueryModel *model);
    bool deletarProdutoVendido(qlonglong id);
    bool deletarPorIdVenda(qlonglong idvenda);
    int contarProdutosVendidosFromVenda(qlonglong idvenda);
    ProdutoVendidoDTO getProdutoVendido(qlonglong id);
    bool inserir(ProdutoVendidoDTO prod);
private:
    QSqlDatabase db;

signals:
};

#endif // PRODUTOVENDA_REPOSITORY_H
