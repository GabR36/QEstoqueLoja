#ifndef PRODUTOVENDA_SERVICE_H
#define PRODUTOVENDA_SERVICE_H

#include <QObject>
#include "../repository/produtovenda_repository.h"
#include <QSqlQueryModel>

enum class ProdutoVendaErro{
    Nenhum,
    Banco,
    Salvar,
    Update,
    Deletar
};

class ProdutoVenda_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        ProdutoVendaErro erro = ProdutoVendaErro::Nenhum;
        QString msg;
    };

    explicit ProdutoVenda_service(QObject *parent = nullptr);
    void listarProdutosVenda(QSqlQueryModel *model);
    QList<ProdutoVendidoDTO> getProdutosVendidos(qlonglong idVenda);
    void listarProdutosVendidosFromVenda(qlonglong idvenda, QSqlQueryModel *model);
    ProdutoVenda_service::Resultado deletarProdutoVendido(qlonglong id);
    ProdutoVenda_service::Resultado deletarProdutosVendidosPorIdVenda(qlonglong idvenda);
    bool temApenasUmProduto(qlonglong idvenda);
    ProdutoVendidoDTO getProdutoVendido(qlonglong id);
    ProdutoVenda_service::Resultado inserir(ProdutoVendidoDTO prod);
private:
    ProdutoVenda_repository prodVendaRepo;

signals:
};

#endif // PRODUTOVENDA_SERVICE_H
