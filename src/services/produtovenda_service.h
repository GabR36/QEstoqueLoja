#ifndef PRODUTOVENDA_SERVICE_H
#define PRODUTOVENDA_SERVICE_H

#include <QObject>
#include "../repository/produtovenda_repository.h"
#include <QSqlQueryModel>

class ProdutoVenda_service : public QObject
{
    Q_OBJECT
public:
    explicit ProdutoVenda_service(QObject *parent = nullptr);
    QSqlQueryModel *listarProdutosVenda();
    QList<ProdutoVendidoDTO> getProdutosVendidos(qlonglong idVenda);
    QSqlQueryModel *listarProdutosVendidosFromVenda(qlonglong idvenda);
private:
    ProdutoVenda_repository prodVendaRepo;
signals:
};

#endif // PRODUTOVENDA_SERVICE_H
