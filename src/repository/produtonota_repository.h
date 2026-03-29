#ifndef PRODUTONOTA_REPOSITORY_H
#define PRODUTONOTA_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QVariantMap>
#include "../dto/ProdutoNota_dto.h"

class ProdutoNota_repository : public QObject
{
    Q_OBJECT
public:
    explicit ProdutoNota_repository(QObject *parent = nullptr);
    bool inserir(ProdutoNotaDTO produtoNota);
    void listarPorNota(QSqlQueryModel *model, qlonglong idNf);
    ProdutoNotaDTO getProdutoNota(qlonglong id);
    QString getXmlPathPorId(qlonglong id);
    bool marcarComoAdicionado(qlonglong id);
    QVariantMap getProdutoNotaComXmlPath(qlonglong id);
    bool marcarComoDevolvido(qlonglong id, qlonglong idNfDevol);
private:
    QSqlDatabase db;

signals:
};

#endif // PRODUTONOTA_REPOSITORY_H
