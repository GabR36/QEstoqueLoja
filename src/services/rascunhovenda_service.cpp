#include "rascunhovenda_service.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

RascunhoVenda_service::RascunhoVenda_service(QObject *parent)
    : QObject{parent}
{
}

bool RascunhoVenda_service::salvar(const RascunhoVendaSaveDTO &saveDto)
{
    QJsonArray arr;
    for (const ProdutoVendidoDTO &p : saveDto.produtos) {
        QJsonObject obj;
        obj["idProduto"]    = p.idProduto;
        obj["quantidade"]   = p.quantidade;
        obj["descricao"]    = p.descricao;
        obj["precoVendido"] = p.precoVendido;
        arr.append(obj);
    }

    RascunhoVendaDTO rascunho;
    rascunho.idCliente           = saveDto.idCliente;
    rascunho.cpfManual           = saveDto.cpfManual;
    rascunho.dataHora            = saveDto.dataHora;
    rascunho.produtosJson        = QString(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    rascunho.formaPagamento      = saveDto.formaPagamento;
    rascunho.desconto            = saveDto.desconto;
    rascunho.taxa                = saveDto.taxa;
    rascunho.recebido            = saveDto.recebido;
    rascunho.descontoPorcentagem = saveDto.descontoPorcentagem;
    rascunho.modeloNf            = saveDto.modeloNf;
    rascunho.emitirTodos         = saveDto.emitirTodos;
    rascunho.atualizadoEm        = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    return rascunhoRepo.salvar(rascunho);
}

QList<ProdutoVendidoDTO> RascunhoVenda_service::carregarProdutos(const QString &produtosJson)
{
    QList<ProdutoVendidoDTO> lista;

    QJsonArray arr = QJsonDocument::fromJson(produtosJson.toUtf8()).array();

    for (const QJsonValue &v : arr) {
        QJsonObject obj = v.toObject();
        ProdutoVendidoDTO p;
        p.idProduto    = obj["idProduto"].toVariant().toLongLong();
        p.quantidade   = obj["quantidade"].toDouble();
        p.precoVendido = obj["precoVendido"].toDouble();
        p.descricao    = obj["descricao"].toString();
        lista.append(p);
    }

    return lista;
}

bool RascunhoVenda_service::descartar()
{
    return rascunhoRepo.descartar();
}

RascunhoVendaDTO RascunhoVenda_service::carregar()
{
    return rascunhoRepo.carregar();
}

bool RascunhoVenda_service::existe()
{
    return rascunhoRepo.existe();
}
