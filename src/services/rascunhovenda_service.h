#ifndef RASCUNHOVENDA_SERVICE_H
#define RASCUNHOVENDA_SERVICE_H

#include <QObject>
#include <QList>
#include "../repository/rascunhovenda_repository.h"
#include "../dto/RascunhoVenda_dto.h"
#include "../dto/RascunhoVendaSave_dto.h"
#include "../dto/ProdutoVendido_dto.h"

class RascunhoVenda_service : public QObject
{
    Q_OBJECT
public:
    explicit RascunhoVenda_service(QObject *parent = nullptr);

    bool salvar(const RascunhoVendaSaveDTO &saveDto);
    bool descartar();
    RascunhoVendaDTO carregar();
    QList<ProdutoVendidoDTO> carregarProdutos(const QString &produtosJson);
    bool existe();

private:
    RascunhoVenda_repository rascunhoRepo;

signals:
};

#endif // RASCUNHOVENDA_SERVICE_H
