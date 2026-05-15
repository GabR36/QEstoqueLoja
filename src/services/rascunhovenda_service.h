#ifndef RASCUNHOVENDA_SERVICE_H
#define RASCUNHOVENDA_SERVICE_H

#include <QObject>
#include <QList>
#include "../repository/rascunhovenda_repository.h"
#include "../dto/RascunhoVenda_dto.h"
#include "../dto/ProdutoVendido_dto.h"

class RascunhoVenda_service : public QObject
{
    Q_OBJECT
public:
    explicit RascunhoVenda_service(QObject *parent = nullptr);

    bool salvar(qlonglong idCliente,
                const QString &cpfManual,
                const QString &dataHora,
                const QList<ProdutoVendidoDTO> &produtos,
                const QString &formaPagamento,
                const QString &desconto,
                const QString &taxa,
                const QString &recebido,
                bool descontoPorcentagem,
                int  modeloNf,
                bool emitirTodos);

    bool descartar();
    RascunhoVendaDTO carregar();
    QList<ProdutoVendidoDTO> carregarProdutos(const QString &produtosJson);
    bool existe();

private:
    RascunhoVenda_repository rascunhoRepo;

signals:
};

#endif // RASCUNHOVENDA_SERVICE_H
