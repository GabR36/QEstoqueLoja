#ifndef MANIFESTADORDFE_H
#define MANIFESTADORDFE_H

#include <QObject>
#include <sstream>
#include <QMap>
#include <QSqlDatabase>
#include "../nota/eventocienciaop.h"
#include "../services/config_service.h"
#include "../services/dfe_service.h"
#include "../infra/databaseconnection_service.h"
#include "../services/notafiscal_service.h"
#include "../dto/NotaFiscal_dto.h"
#include <qlocale.h>
#include "../services/produtonota_service.h"
#include "../util/nfxmlutil.h"
#include "../services/cliente_service.h"
#include "../services/eventofiscal_service.h"


class ManifestadorDFe : public QObject
{
    Q_OBJECT
public:
    explicit ManifestadorDFe(QObject *parent = nullptr);
    bool enviarCienciaOperacao(const QString &chNFe, const QString &cnpjEmit);
    void consultarEManifestar();
    void consultarEBaixarXML();
    void consultaAlternada();
    void consultarSePossivel();
private:
    QSqlDatabase db;
    ConfigDTO configDTO;
    QString cuf,cnpj;
    QString ultimo_nsu;
    QString ultNsuXml;
    QString novoUltNsuXml;
    Dfe_service dfeServ;
    NotaFiscal_service nfServ;
    QLocale portugues;
    ProdutoNota_service prodNotaServ;
    NfXmlUtil xmlUtil;
    Cliente_service cliServ;
    EventoFiscal_service eveServ;



    void salvarEventoNoBanco(EventoFiscalDTO info, const QString &chaveNFe);
    void carregarConfigs();
    void processarHeaderDfe(const QString &bloco);
    void salvarNovoUltNsu(const QString &ultNsu);
    bool existeCienciaOperacao(const QString &chNFe);
    void salvarResumoNota(NotaFiscalDTO resumo);
    void processarResumo(const QString &bloco);
    void processarNota(const QString &bloco);
    bool salvarEmitenteCliente(NotaFiscalDTO notaInfo);
    bool atualizarNotaBanco(NotaFiscalDTO notaInfo);
    void processarHeaderDfeXML(const QString &bloco);
    bool salvarProdutosNota(const QString &xml_path, const QString &chnfe);
signals:
};

#endif // MANIFESTADORDFE_H
