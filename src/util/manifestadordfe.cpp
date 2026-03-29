#include "manifestadordfe.h"
#include <QRegularExpression>
#include "../nota/acbrmanager.h"
#include "../nota/eventocienciaop.h"
#include <QSqlQuery>
#include <QDateTime>
#include <QDebug>
#include "../configuracao.h"
#include <QFile>
#include <QDomDocument>
#include <QDomNode>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include "../services/acbr_service.h"
#include "../dto/ProdutoNota_dto.h"


ManifestadorDFe::ManifestadorDFe(QObject *parent)
    : QObject{parent}
{
    db = DatabaseConnection_service::db();
    Config_service confServ;
    configDTO = confServ.carregarTudo();

    portugues = QLocale(QLocale::Portuguese, QLocale::Brazil);
    cnpj = configDTO.cnpjEmpresa;
    cuf = configDTO.cUfFiscal;
    carregarConfigs();

}
void ManifestadorDFe::carregarConfigs(){
    if(retornoForcado.isEmpty()){
        Acbr_service acbrServ;
        acbrServ.carregarConfigParaDFE();
    }

}

void ManifestadorDFe::consultaAlternada(){
    QString ultimaAcao = dfeServ.getUltimaIdentificaçãoUsada();


    if (ultimaAcao == "consulta_xml") {
        consultarEManifestar();      // próxima ação
    } else if (ultimaAcao == "consulta_resumo") {
        consultarEBaixarXML();       // próxima ação
    } else {
        // primeiro uso ou inconsistente
        qDebug() << "Nao encontou ultima ação DFEINFO";
        consultarEManifestar();
    }

}

void ManifestadorDFe::setRetornoForcado(const QString& retorno)
{
    retornoForcado = retorno;
}


void ManifestadorDFe::consultarEManifestar(){
    qDebug() << "consultar rodou";
   //teste com arquivo simulador de retorno
    // QFile file("retorno_dfe.txt");
    // QString retorno;

    // if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //     retorno = QString::fromUtf8(file.readAll());
    //     file.close();
    // } else {
    //     qDebug() << "Erro ao abrir retorno.txt";
    // }
    // qDebug() << retorno;

    // auto acbr = AcbrManager::instance()->nfe();
    ultimo_nsu = dfeServ.getUltNsuResumo();
    // qDebug() << "ultimo nsu achado no banco:" << ultimo_nsu;
    // std::string retorno = acbr->DistribuicaoDFePorUltNSU(cuf.toInt(), cnpj.toStdString(), ultimo_nsu.toStdString());
    // qDebug() << "Retorno consulta DFE" << retorno;
    // // QString whole = QString::fromStdString(retorno.toStdString());
    // QString whole = QString::fromStdString(retorno);


    QString whole;

    if (!retornoForcado.isEmpty()) {
        whole = retornoForcado;
    } else {
        auto acbr = AcbrManager::instance()->nfe();
        whole = QString::fromStdString(
            acbr->DistribuicaoDFePorUltNSU(cuf.toInt(), cnpj.toStdString(), ultimo_nsu.toStdString())
            );
        qDebug() << "Retorno consulta DFE" << whole;

    }

    QStringList blocos = whole.split("[", Qt::SkipEmptyParts);

    for (QString bloco : blocos)
    {
        bloco = "[" + bloco;

        if (bloco.startsWith("[ResNFe") || bloco.startsWith("[ResDFe"))
            processarResumo(bloco);
    }
    for (QString bloco : blocos)
    {
        bloco = "[" + bloco;

        if (bloco.startsWith("[DistribuicaoDFe"))
            processarHeaderDfe(bloco);
    }
}

void ManifestadorDFe::consultarEBaixarXML(){
    qDebug() << "rodou consultarEBaixarXML()";

    QString whole;
    if(!retornoForcado.isEmpty()){
         whole = retornoForcado;
    }else{

        auto acbr = AcbrManager::instance()->nfe();
        ultNsuXml = dfeServ.getUltNsuXml();
        std::string retorno = acbr->DistribuicaoDFePorUltNSU(cuf.toInt(), cnpj.toStdString(), ultNsuXml.toStdString());
        qDebug() << "Retorno consulta DFE" << retorno;
        qDebug() << "ult nsuxml: " << ultNsuXml;
        whole = QString::fromStdString(retorno);

    }

    QStringList blocos = whole.split("[", Qt::SkipEmptyParts);
    novoUltNsuXml = "0";
    for (QString bloco : blocos)
    {
        bloco = "[" + bloco;

        if (bloco.startsWith("[ResNFe") || bloco.startsWith("[ResDFe"))
            processarNota(bloco);
    }
    for (QString bloco : blocos)
    {
        bloco = "[" + bloco;

        if (bloco.startsWith("[DistribuicaoDFe"))
            processarHeaderDfeXML(bloco);
    }
    if(novoUltNsuXml.toLongLong() > ultNsuXml.toLongLong()){
        dfeServ.salvarNovoUltNsuXml(novoUltNsuXml);
    }

}

void ManifestadorDFe::processarHeaderDfe(const QString &bloco){

    auto campo = [&](QString nome) {
        QRegularExpression re(nome + R"(=([^\r\n]*))");
        QRegularExpressionMatch m = re.match(bloco);
        return m.hasMatch() ? m.captured(1).trimmed() : "";
    };

    QString cStat = campo("CStat");
    QString msg   = campo("Msg");
    QString ultNsu = campo("ultNSU");

    // Limpeza de NSU como já tinha
    ultNsu.replace("\\n", "");
    ultNsu.replace("\\r", "");
    ultNsu.remove('\n');
    ultNsu.remove('\r');
    ultNsu = ultNsu.trimmed();

    qDebug() << "CStat da consulta:" << cStat;
    qDebug() << "Mensagem:" << msg;
    qDebug() << "ultNSU:" << ultNsu;

    // nÃO atualizar o novo NSU se CStat indicar erro

    if (cStat != "138") {
        qDebug() << "Consulta retornou erro, não atualizou ultNSU!";
        dfeServ.atualizarDataNsu(TipoDfeInfo::ConsultaResumo);// 1 = atualiza consulta_resumo
        return;
    }

    //somente para tirar os 00000 da frente no numero
    ultNsu = QString::number(ultNsu.toLongLong());

    // Somente sucesso > atualiza NSU
    dfeServ.salvarNovoUltNsuResumo(ultNsu);
}

void ManifestadorDFe::processarHeaderDfeXML(const QString &bloco){

    auto campo = [&](QString nome) {
        QRegularExpression re(nome + R"(=([^\r\n]*))");
        QRegularExpressionMatch m = re.match(bloco);
        return m.hasMatch() ? m.captured(1).trimmed() : "";
    };

    QString cStat = campo("CStat");
    QString msg   = campo("Msg");
    QString ultNsu = campo("ultNSU");

    // Limpeza de NSU como já tinha
    ultNsu.replace("\\n", "");
    ultNsu.replace("\\r", "");
    ultNsu.remove('\n');
    ultNsu.remove('\r');
    ultNsu = ultNsu.trimmed();

    qDebug() << "CStat da consulta:" << cStat;
    qDebug() << "Mensagem:" << msg;
    qDebug() << "ultNSU:" << ultNsu;

    // nÃO atualizar o novo NSU se CStat indicar erro

    if (cStat != "138") {
        qDebug() << "Consulta retornou erro, não atualizou ultNSU!";
        dfeServ.atualizarDataNsu(TipoDfeInfo::ConsultaXml);// 2 = atualiza consulta_xml
        return;
    }
    dfeServ.atualizarDataNsu(TipoDfeInfo::ConsultaXml);
    // // Somente sucesso > atualiza NSU
    // salvarNovoUltNsu(ultNsu);
}

void ManifestadorDFe::salvarResumoNota(NotaFiscalDTO resumo){
    auto resultado = nfServ.salvarResNfe(resumo);

    if(!resultado.ok){
        qDebug() << resultado.msg;
    } else {
        qDebug() << "Resumo nota salvo com sucesso!";
    }
}

void ManifestadorDFe::processarResumo(const QString &bloco)
{
    auto campo = [&](const QString &nome) {
        // ^nome=(valor) em qualquer linha do bloco
        QRegularExpression re("^" + nome + R"(=(.*))",
                              QRegularExpression::MultilineOption);

        auto it = re.globalMatch(bloco);
        if (!it.hasNext())
            return QString();

        QString value = it.next().captured(1).trimmed();
        return value;
    };


    NotaFiscalDTO resumo;
    resumo.chNfe      = campo("chDFe");
    // resumo.nome     = campo("xNome");
    QString nome     = campo("xNome");
    resumo.cnpjEmit   = campo("CNPJCPF");
    resumo.finalidade = campo("schema");
    QString vnf = campo("vNF");
    resumo.cstat      = campo("CStat");
    resumo.nProt      = campo("nProt");
    resumo.dhEmi      = campo("dhEmi");
    resumo.tpAmb = 1; //NOVO refactoring
    qDebug() << "Valor total resumo: " << vnf;
    resumo.valorTotal = portugues.toDouble(vnf);

    QString path = campo("arquivo");

    path.remove('\r');
    path.remove('\n');

    path.replace('\\', '/');

    // Remove duplicações tipo //
    path = QDir::cleanPath(path);

    resumo.xmlPath = path;

    // Apenas resumos válidos
    if (!resumo.finalidade.contains("resNFe"))
        return;

    qDebug() << "\nResumo localizado:" << resumo.chNfe << nome;

    salvarResumoNota(resumo);
    if(!retornoForcado.isEmpty()){
        enviarCienciaOperacao(resumo.chNfe, resumo.cnpjEmit);
    }
}

void ManifestadorDFe::processarNota(const QString &bloco)
{
    auto campoTexto = [&](const QString &nome) {
        // Captura até o fim da linha, com limpeza total
        QRegularExpression re("^" + nome + R"(=(.*)$)",
                              QRegularExpression::MultilineOption);
        auto m = re.match(bloco);
        if (!m.hasMatch()) return QString();

        QString v = m.captured(1).trimmed();
        v.remove("\r");
        v.remove("\n");
        return v;
    };

    auto campoNumero = [&](const QString &nome) {
        // Captura somente até o fim da linha (seguro mesmo com XML= depois)
        QRegularExpression re(nome + R"(=([^\r\n]+))");
        auto m = re.match(bloco);
        return m.hasMatch() ? m.captured(1).trimmed() : QString();
    };


    NotaFiscalDTO nfe;
    nfe.chNfe    = campoTexto("chDFe");
    // nfe.nome     = campoTexto("xNome");
    QString nome     = campoTexto("xNome");
    nfe.cnpjEmit = campoTexto("CNPJCPF");
    nfe.finalidade   = campoTexto("schema");
    QString vnf      = campoTexto("vNF");
    nfe.cstat    = campoTexto("CStat");
    nfe.nProt    = campoTexto("nProt");
    nfe.dhEmi    = campoTexto("dhEmi");
    QString cSitNfe  = campoTexto("cSitNFe");
    nfe.valorTotal = portugues.toDouble(vnf);

    QString path = campoTexto("arquivo");

    path.remove('\r');
    path.remove('\n');

    path.replace('\\', '/');

    // Remove duplicações tipo //
    path = QDir::cleanPath(path);

    nfe.xmlPath = path;

    // nfe.nsu      = campoNumero("NSU"); //--**
    QString nsu      = campoNumero("NSU");


    // Não processa resumos inválidos
    if (!nfe.finalidade.contains("procNFe"))
        return;

    qDebug() << "\n+++ Nota XML localizado +++";
    qDebug() << "Chave:" << nfe.chNfe;
    qDebug() << "NSU capturado:" << nsu;
    qDebug() << "Emitente:" << nome;

    if (cSitNfe == "1")
    {
        if (salvarEmitenteCliente(nfe) && atualizarNotaBanco(nfe))
        {
            qlonglong nsuInt = nsu.toLongLong();

            if (nsuInt > novoUltNsuXml.toLongLong())
            {
                novoUltNsuXml = QString::number(nsuInt);
                qDebug() << "Novo ultNSU atualizado para:" << novoUltNsuXml;
            }
            if(salvarProdutosNota(nfe.xmlPath, nfe.chNfe)){
                qDebug() << "Salvou os produtos da nota";
            }else{
                qDebug() << "Não salvou os produtos da nota";
            }
        }
        else
        {
            qDebug() << "Erro ao atualizar emitente ou nota.";
        }
    }
}

bool ManifestadorDFe::salvarProdutosNota(const QString &xml_path, const QString &chnfe){
    qlonglong id_nf = nfServ.getIdFromChave(chnfe);

    QList<ProdutoNotaDTO> produtos = xmlUtil.carregarProdutosDaNFe(xml_path, id_nf);

    auto result = prodNotaServ.inserirListaProdutos(produtos);
    if(!result.ok){
        return false;
    }else{
        return true;
    }
}

bool ManifestadorDFe::atualizarNotaBanco(NotaFiscalDTO notaInfo){
    NotaFiscalDTO nf = xmlUtil.lerNotaFiscalDoXML(notaInfo.xmlPath);
    qDebug() << "XML PATH atualizar NOTA BANCO: " << notaInfo.xmlPath;
    qlonglong idcliente;
    idcliente = cliServ.getIdFromCpfCnpj(nf.cnpjEmit);

    nf.idEmissorCliente = idcliente;

    auto r1 = nfServ.updateWhereChave(nf, notaInfo.chNfe);
    if(!r1.ok){
        qDebug() << r1.msg;
        return false;
    }else{
        return true;
    }
}

bool ManifestadorDFe::salvarEmitenteCliente(NotaFiscalDTO notaInfo){

    ClienteDTO emi;

    emi = xmlUtil.getEmitenteFromXML(notaInfo.xmlPath);

    qDebug() << "CNPJ a ser adicionado/buscado" << emi.cpf;
    qlonglong total = cliServ.contarQuantosRegistrosPorCPFCNPJ(emi.cpf);


    if (total > 0) {
        qDebug() << "Cliente já cadastrado com este CNPJ/CPF!";
        return true;  // evita inserir duplicado
    }

    auto result = cliServ.inserirClienteEmitente(emi);

    if(!result.ok){
        qDebug() << "Query insert salvarEmitenteCliente nao funcionou!";
        return false;
    }else{
        qDebug() << "Fornecedor adicionado com sucesso!";
        return true;
    }

}


bool ManifestadorDFe::enviarCienciaOperacao(const QString &chNFe, const QString &cnpjEmit)
{

    EventoCienciaOP *evento = new EventoCienciaOP(this, chNFe);


    // Envia
    EventoFiscalDTO info =  evento->gerarEnviarRetorno();
    if(info.cstat == "128" || info.cstat == "135" || info.cstat == "136"){
        salvarEventoNoBanco(info, chNFe);
    }
    qDebug() << "Manifestacao enviada:";

    bool sucesso = info.cstat == "128" || info.cstat == "135" || info.cstat == "136";
    return sucesso;
}

void ManifestadorDFe::salvarEventoNoBanco(EventoFiscalDTO info, const QString &chaveNFe)
{

    qlonglong idnf;

    idnf = nfServ.getIdFromChave(chaveNFe);
    EventoFiscalDTO evento;
    evento = info;
    evento.idNf = idnf;

    auto result = eveServ.inserir(evento);

    if(!result.ok){
        qDebug() << result.msg;
    }
}

void ManifestadorDFe::consultarSePossivel(){
    if(dfeServ.possoConsultar() &&
        configDTO.emitNfFiscal && configDTO.tpAmbFiscal){

        try {
            consultarEManifestar();
        }
        catch (const std::exception &e) {
            qDebug() << "Erro ao consultar DFE:" << e.what();
        }
        catch (...) {
            qDebug() << "Erro desconhecido ao consultar DFE";
        }
    }else{
        qDebug() << "Nao consultado DFE";

    }
}
