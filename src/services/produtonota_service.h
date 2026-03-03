#ifndef PRODUTONOTA_SERVICE_H
#define PRODUTONOTA_SERVICE_H

#include <QObject>
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
private:
    ProdutoNota_repository prodNotaRepo;

signals:
};

#endif // PRODUTONOTA_SERVICE_H
