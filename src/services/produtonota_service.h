#ifndef PRODUTONOTA_SERVICE_H
#define PRODUTONOTA_SERVICE_H

#include <QObject>
#include <QSqlQueryModel>
#include <QVariantMap>
#include "../dto/ProdutoNota_dto.h"
#include "../repository/produtonota_repository.h"

enum class ProdutoNotaErro {
    Nenhum,
    CampoVazio,
    InsercaoInvalida
};

class ProdutoNota_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        ProdutoNotaErro erro = ProdutoNotaErro::Nenhum;
        QString msg;
    };
    explicit ProdutoNota_service(QObject *parent = nullptr);
    ProdutoNota_service::Resultado inserirListaProdutos(QList<ProdutoNotaDTO> lista);
    void listarPorNota(QSqlQueryModel *model, qlonglong idNf);
    ProdutoNotaDTO getProdutoNota(qlonglong id);
    QString getXmlPathPorId(qlonglong id);
    bool marcarComoAdicionado(qlonglong id);
    QVariantMap getProdutoNotaComXmlPath(qlonglong id);
    bool marcarComoDevolvido(qlonglong id, qlonglong idNfDevol);
private:
    ProdutoNota_repository prodNotaRepo;

signals:
};

#endif // PRODUTONOTA_SERVICE_H
