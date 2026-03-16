#ifndef FISCALEMITTER_SERVICE_H
#define FISCALEMITTER_SERVICE_H

#include <QObject>
#include "../nota/nfeacbr.h"
#include "../dto/ProdutoVendido_dto.h"
#include "notafiscal_service.h"
#include "../dto/Vendas_dto.h"
#include "Produto_service.h"
#include "../nota/nfceacbr.h"
#include "../dto/Cliente_dto.h"
#include "email_service.h"
#include "config_service.h"

enum class FiscalEmitterErro{
    Nenhum,
    Banco,
    Salvar,
    Update,
    Inserir,
    ErroAoEnviar,
    Recusado,
    InfoInsuficiente,
    ProdutosSemNF,
    QuebraDeRegra,
    NCMInvalido
};

class FiscalEmitter_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        FiscalEmitterErro erro = FiscalEmitterErro::Nenhum;
        QString msg;
    };

    // struct ResultadoNFe {
    //     bool ok = false;
    //     FiscalEmitterErro erro = FiscalEmitterErro::Nenhum;
    //     QString msg = "ERRO";
    //     QString xmlPath = "";
    //     std::string pdfDanfe = "";
    // };
    explicit FiscalEmitter_service(QObject *parent = nullptr);
    void setRetornoForcado(const QString &retorno);
    FiscalEmitter_service::Resultado enviarNfeDevolucaoPadrao(qlonglong idvenda, QList<ProdutoVendidoDTO> listaProds);
    FiscalEmitter_service::Resultado enviarNfcePadrao(VendasDTO venda, QList<ProdutoVendidoDTO> listaProds,
                                                      qlonglong nnf, ClienteDTO cliente, bool emitirTodos,
                                                      bool ignorarNCM);
    FiscalEmitter_service::Resultado enviarNFePadrao(VendasDTO venda, QList<ProdutoVendidoDTO> listaProds, qlonglong nnf, ClienteDTO cliente, bool emitirTodos, bool ignorarNCM);
private:
    QString retornoForcado = "";
    NotaFiscal_service notaServ;
    Produto_Service prodServ;
    Email_service emailServ;
    Config_service confServ;
    ConfigDTO confDTO;


signals:
};

#endif // FISCALEMITTER_SERVICE_H
