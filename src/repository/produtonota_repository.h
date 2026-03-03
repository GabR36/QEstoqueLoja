#ifndef PRODUTONOTA_REPOSITORY_H
#define PRODUTONOTA_REPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include "../dto/ProdutoNota_dto.h"

class ProdutoNota_repository : public QObject
{
    Q_OBJECT
public:
    explicit ProdutoNota_repository(QObject *parent = nullptr);
    bool inserir(ProdutoNotaDTO produtoNota);
private:
    QSqlDatabase db;

signals:
};

#endif // PRODUTONOTA_REPOSITORY_H
