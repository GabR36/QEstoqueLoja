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

ManifestadorDFe::ManifestadorDFe(QObject *parent)
    : QObject{parent}
{
    db = QSqlDatabase::database();
    empresaValues = Configuracao::get_All_Empresa_Values();
    fiscalValues = Configuracao::get_All_Fiscal_Values();
    cnpj = empresaValues.value("cnpj_empresa");
    cuf = fiscalValues.value("cuf");
    carregarConfigs();
}
void ManifestadorDFe::carregarConfigs(){
    auto acbr = AcbrManager::instance()->nfe();
    acbr->ConfigGravarValor("NFe", "ModeloDF", "0");
}
void ManifestadorDFe::consultaAlternada(){
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "Banco nao abriu consultaAlternada";
        }
    }
    QSqlQuery q("SELECT identificacao FROM dfe_info ORDER BY datetime(data_modificado) DESC LIMIT 1");

    QString ultimaAcao;

    if (q.next()) {
        ultimaAcao = q.value(0).toString();
    }

    if (ultimaAcao == "consulta_xml") {
        consultarEManifestar();      // próxima ação
    } else if (ultimaAcao == "consulta_resumo") {
        consultarEBaixarXML();       // próxima ação
    } else {
        // primeiro uso ou inconsistente
        consultarEManifestar();
    }

}
QString ManifestadorDFe::getUltNsu(){
    if(!db.open()){
        qDebug() << "não abriu bd getultnsu";
    }
    QSqlQuery query;
    query.exec("SELECT ult_nsu FROM dfe_info WHERE identificacao = 'consulta_resumo'");

    QString ultnsu;
    while(query.next()){
        ultnsu = query.value(0).toString();
    }
    return ultnsu;
}
QString ManifestadorDFe::getUltNsuXml(){
    if(!db.open()){
        qDebug() << "não abriu bd getultnsu";
    }
    QSqlQuery query;
    query.exec("SELECT ult_nsu FROM dfe_info WHERE identificacao = 'consulta_xml'");

    QString ultnsu;
    while(query.next()){
        ultnsu = query.value(0).toString();
    }
    return ultnsu;
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

    auto acbr = AcbrManager::instance()->nfe();
    ultimo_nsu = getUltNsu();
    qDebug() << "ultimo nsu achado no banco:" << ultimo_nsu;
    std::string retorno = acbr->DistribuicaoDFePorUltNSU(cuf.toInt(), cnpj.toStdString(), ultimo_nsu.toStdString());
    qDebug() << "Retorno consulta DFE" << retorno;
    // QString whole = QString::fromStdString(retorno.toStdString());
    QString whole = QString::fromStdString(retorno);


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
    auto acbr = AcbrManager::instance()->nfe();
    ultNsuXml = getUltNsuXml();
    std::string retorno = acbr->DistribuicaoDFePorUltNSU(cuf.toInt(), cnpj.toStdString(), ultNsuXml.toStdString());
    qDebug() << "Retorno consulta DFE" << retorno;
    qDebug() << "ult nsuxml: " << ultNsuXml;

    // QFile file("retorno_distribuicao2.txt");
    // QString retorno;

    // if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //     retorno = QString::fromUtf8(file.readAll());
    //     file.close();
    // } else {
    //     qDebug() << "Erro ao abrir retorno.txt";
    // }

    QString whole = QString::fromStdString(retorno);

    // // --- SALVAR EM ARQUIVO TXT ---
    // QFile file("retorno_distribuicao2.txt");
    // if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    //     QTextStream out(&file);
    //     out << whole;
    //     file.close();
    //     qDebug() << "Arquivo salvo com sucesso!";
    // } else {
    //     qDebug() << "Erro ao salvar o arquivo!";
    // }
    // -----------------------------
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
        salvarNovoUltNsuXml(novoUltNsuXml);
    }


}

void ManifestadorDFe::salvarNovoUltNsuXml(const QString &ultnsuxml){
    if (!db.open()) {
        qDebug() << "Erro ao abrir DB em salvarNovoUltNsu:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    QDateTime dataIngles = QDateTime::currentDateTime();
    QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");

    query.prepare("UPDATE dfe_info SET ult_nsu = :nsu, data_modificado = :datamod "
                  "WHERE identificacao = 'consulta_xml'");
    query.bindValue(":nsu", ultnsuxml);
    query.bindValue(":datamod", dataFormatada);

    if(!query.exec()){
        qDebug() << "query update ultnsuxml falhou:" << query.lastError().text();
        qDebug() << "SQL:" << query.lastQuery();
    } else {
        qDebug() << "query update ultnsuxml rodou ok";
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
        atualizarDataNsu(1);// 1 = atualiza consulta_resumo
        return;
    }

    // Somente sucesso > atualiza NSU
    salvarNovoUltNsu(ultNsu);
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
        atualizarDataNsu(2);// 2 = atualiza consulta_xml
        return;
    }
    atualizarDataNsu(2);
    // // Somente sucesso > atualiza NSU
    // salvarNovoUltNsu(ultNsu);
}

void ManifestadorDFe::atualizarDataNsu(int option){
    if (!db.open()) {
        qDebug() << "Erro ao abrir DB em atualizarDataNsu:" << db.lastError().text();
        return;
    }
    qDebug() << "atualizando data modificado nsu";
    QString identificacao;
    if(option == 1 ){
        identificacao = "consulta_resumo";
    }else{
        identificacao = "consulta_xml";
    }

    QDateTime dataIngles = QDateTime::currentDateTime();
    QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");
    QSqlQuery query(db);
    query.prepare("UPDATE dfe_info SET data_modificado = :datamod "
                  "WHERE identificacao = :ide");
    query.bindValue(":datamod", dataFormatada );
    query.bindValue(":ide", identificacao);
    if(!query.exec()){
        qDebug() << "nao completou query atualizar data";
    }
}

void ManifestadorDFe::salvarNovoUltNsu(const QString &ultNsu){
    if (!db.open()) {
        qDebug() << "Erro ao abrir DB em salvarNovoUltNsu:" << db.lastError().text();
        return;
    }
    QDateTime dataIngles = QDateTime::currentDateTime();
    QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");

    QSqlQuery query(db);
    query.prepare("UPDATE dfe_info SET ult_nsu = :nsu, data_modificado = :datamod "
                  "WHERE identificacao = 'consulta_resumo'");
    query.bindValue(":nsu", ultNsu);
    query.bindValue(":datamod", dataFormatada);

    if(!query.exec()){
        qDebug() << "query update ultnsu falhou:" << query.lastError().text();
        qDebug() << "SQL:" << query.lastQuery();
    } else {
        qDebug() << "query update ultnsu rodou ok";
    }

}

void ManifestadorDFe::salvarResumoNota(ResumoNFe resumo){
    if(!db.isOpen()){
        qDebug() << "db nao aberto ao salvar resumo nota";
    }
    QDateTime dataIngles = QDateTime::currentDateTime();
    QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");
    QLocale portugues(QLocale::Portuguese, QLocale::Brazil);
    QSqlQuery query;
    query.prepare("INSERT INTO notas_fiscais (cstat, modelo, tp_amb, xml_path, valor_total, atualizado_em,"
                  "cnpjemit, chnfe, nprot, cuf, finalidade, saida, nnf, serie) VALUES (:cstat, :modelo,"
                  " :tpamb, :xmlpath, :vnf, :atualizadoem, :cnpjemit, :chnf, :nprot, :cuf, :finalidade, "
                  ":saida, :nnf, :serie)");
    query.bindValue(":cstat", resumo.cstat);
    query.bindValue(":modelo", "55");
    query.bindValue(":tpamb", fiscalValues.value("tp_amb"));
    query.bindValue(":xmlpath", resumo.xml_path);
    query.bindValue(":vnf", portugues.toString(resumo.vnf.toDouble()));
    query.bindValue(":atualizadoem", dataFormatada);
    query.bindValue(":cnpjemit", resumo.cnpjEmit);
    query.bindValue(":chnf", resumo.chave);
    query.bindValue(":nprot", resumo.nProt);
    query.bindValue(":cuf", "");
    query.bindValue(":finalidade", resumo.schema);
    query.bindValue(":saida", "0");
    query.bindValue(":nnf", "0");
    query.bindValue(":serie", "");

    if(!query.exec()){
        qDebug() << "ERRO INSERT notas_fiscais:" << query.lastError().text();
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


    ResumoNFe resumo;
    resumo.chave    = campo("chDFe");
    resumo.nome     = campo("xNome");
    resumo.cnpjEmit = campo("CNPJCPF");
    resumo.schema   = campo("schema");
    resumo.vnf      = campo("vNF");
    resumo.cstat    = campo("CStat");
    resumo.xml_path = campo("arquivo");
    resumo.nProt    = campo("nProt");
    resumo.dhEmi    = campo("dhEmi");

    // Apenas resumos válidos
    if (!resumo.schema.contains("resNFe"))
        return;

    qDebug() << "\nResumo localizado:" << resumo.chave << resumo.nome;

    salvarResumoNota(resumo);
    enviarCienciaOperacao(resumo.chave, resumo.cnpjEmit);
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
        // Somente dígitos impede valores inválidos
        QRegularExpression re("^" + nome + R"(=(\d+)$)",
                              QRegularExpression::MultilineOption);
        auto m = re.match(bloco);
        return m.hasMatch() ? m.captured(1).trimmed() : QString();
    };

    ProcNfe nfe;
    nfe.chave    = campoTexto("chDFe");
    nfe.nome     = campoTexto("xNome");
    nfe.cnpjEmit = campoTexto("CNPJCPF");
    nfe.schema   = campoTexto("schema");
    nfe.vnf      = campoTexto("vNF");
    nfe.cstat    = campoTexto("CStat");
    nfe.xml_path = campoTexto("arquivo");
    nfe.nProt    = campoTexto("nProt");
    nfe.dhEmi    = campoTexto("dhEmi");
    nfe.cSitNfe  = campoTexto("cSitNFe");

    nfe.nsu      = campoNumero("NSU");

    // Não processa resumos inválidos
    if (!nfe.schema.contains("procNFe"))
        return;

    qDebug() << "\n+++ Nota XML localizado +++";
    qDebug() << "Chave:" << nfe.chave;
    qDebug() << "NSU capturado:" << nfe.nsu;
    qDebug() << "Emitente:" << nfe.nome;

    if (nfe.cSitNfe == "1")
    {
        if (salvarEmitenteCliente(nfe) && atualizarNotaBanco(nfe))
        {
            qlonglong nsuInt = nfe.nsu.toLongLong();

            if (nsuInt > novoUltNsuXml.toLongLong())
            {
                novoUltNsuXml = QString::number(nsuInt);
                qDebug() << "Novo ultNSU atualizado para:" << novoUltNsuXml;
            }
            if(salvarProdutosNota(nfe.xml_path, nfe.chave)){
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
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "bd nao abriu salvarProdutosNota()";
            return false;
        }
    }

    QSqlQuery query;
    query.prepare("SELECT id FROM notas_fiscais WHERE chnfe = :chnfe");
    query.bindValue(":chnfe", chnfe);
    QString id_nf = "";
    if(query.exec()){
        while(query.next()){
            id_nf = query.value(0).toString();
        }
    }else{
        qDebug() << "nao rodou query select idnf";
        return false;
    }

    QList<ProdutoNota> produtos = carregarProdutosDaNFe(xml_path, id_nf.toLongLong());

    if (produtos.isEmpty()) {
        qDebug() << "Nenhum produto encontrado no XML";
        return false;
    }

    QSqlQuery ins;
    ins.prepare(
        "INSERT INTO produtos_nota "
        "(id_nf, nitem, quantidade, descricao, preco, codigo_barras, un_comercial, "
        " ncm, csosn, pis, cfop, aliquota_imposto, cst_icms, tem_st, status, adicionado) "
        "VALUES "
        "(:id_nf, :nitem, :quant, :desc, :preco, :cod_barras, :un_comercial, "
        " :ncm, :csosn, :pis, :cfop, :aliquota, :cst_icms, :tem_st, :status, :adicionado)"
        );

    for (const ProdutoNota &p : produtos) {
        ins.bindValue(":id_nf",        p.id_nf);
        ins.bindValue(":nitem",        p.nitem);
        ins.bindValue(":quant",        p.quant);
        ins.bindValue(":desc",         p.desc);
        ins.bindValue(":preco",        p.preco);
        ins.bindValue(":cod_barras",   p.cod_barras);
        ins.bindValue(":un_comercial", p.un_comercial);
        ins.bindValue(":ncm",          p.ncm);
        ins.bindValue(":csosn",        p.csosn);
        ins.bindValue(":pis",          p.pis);
        ins.bindValue(":cfop",         p.cfop);
        ins.bindValue(":aliquota",     p.aliquota_imposto);
        ins.bindValue(":cst_icms",     p.cst_icms);
        ins.bindValue(":tem_st",       p.tem_st ? 1 : 0);
        ins.bindValue(":status",       "OK");
        ins.bindValue(":adicionado", 0);

        if (!ins.exec()) {
            qDebug() << "Erro ao inserir produto:" << ins.lastError().text();
            return false;
        }
    }

    return true;
}


QList<ProdutoNota> ManifestadorDFe::carregarProdutosDaNFe(const QString &xml_path, qlonglong id_nf)
{
    QList<ProdutoNota> lista;

    QFile file(xml_path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Erro ao abrir arquivo XML:" << xml_path;
        return lista;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qDebug() << "Erro ao ler conteúdo do XML";
        file.close();
        return lista;
    }
    file.close();

    QDomNodeList itens = doc.elementsByTagName("det");

    for (int i = 0; i < itens.count(); i++) {
        QDomElement det = itens.at(i).toElement();

        ProdutoNota p;
        p.id_nf = id_nf;

        p.nitem = det.attribute("nItem");

        QDomElement prod = det.firstChildElement("prod");

        p.desc          = prod.firstChildElement("xProd").text();
        p.cod_barras    = prod.firstChildElement("cEAN").text();
        p.un_comercial  = prod.firstChildElement("uCom").text();
        p.ncm           = prod.firstChildElement("NCM").text();
        p.cfop          = prod.firstChildElement("CFOP").text();
        p.quant         = prod.firstChildElement("qCom").text().replace(",", ".").toFloat();
        p.preco         = prod.firstChildElement("vUnCom").text().replace(",", ".").toDouble();

        // ======= IMPOSTOS =======
        QDomElement imposto = det.firstChildElement("imposto");

        // -------------------- CSOSN ou CST ICMS --------------------
        p.csosn = "";
        p.cst_icms = "";
        p.tem_st = false;

        QDomElement icms = imposto.firstChildElement("ICMS");

        QStringList gruposICMS = {
            "ICMS00","ICMS10","ICMS20","ICMS30","ICMS40","ICMS51","ICMS60","ICMS70","ICMS90",
            "ICMSSN101","ICMSSN102","ICMSSN201","ICMSSN202","ICMSSN500","ICMSSN900"
        };

        for (const QString &g : gruposICMS) {
            QDomElement e = icms.firstChildElement(g);
            if (!e.isNull()) {

                // pega CST ou CSOSN
                QString CST = e.firstChildElement("CST").text();
                QString CSOSN = e.firstChildElement("CSOSN").text();

                if (!CSOSN.isEmpty())
                    p.csosn = CSOSN;

                if (!CST.isEmpty())
                    p.cst_icms = CST;

                // identifica ST
                if (g == "ICMS10" || g == "ICMS30" || g == "ICMS60" ||
                    g == "ICMS70" || g == "ICMSSN201" || g == "ICMSSN202") {
                    p.tem_st = true;
                }

                // valida ST por valores
                if (!e.firstChildElement("vBCST").isNull() ||
                    !e.firstChildElement("vICMSST").isNull() ||
                    !e.firstChildElement("vBCSTRet").isNull() ||
                    !e.firstChildElement("vICMSSTRet").isNull())
                {
                    p.tem_st = true;
                }

                break;
            }
        }

        // -------------------- PIS --------------------
        QDomNodeList listaPIS = imposto.elementsByTagName("PISOutr");
        if (listaPIS.isEmpty())
            listaPIS = imposto.elementsByTagName("PISNT");

        if (!listaPIS.isEmpty())
            p.pis = listaPIS.at(0).firstChildElement("CST").text();
        else
            p.pis = "";

        QString aliquota = listaPIS.at(0).firstChildElement("pPIS").text();
        p.aliquota_imposto = aliquota.replace(",", ".").toFloat();

        lista.append(p);
    }

    return lista;
}

bool ManifestadorDFe::atualizarNotaBanco(ProcNfe notaInfo){
    NotaFiscal nf = lerNotaFiscalDoXML(notaInfo.xml_path);
    qDebug() << "XML PATH atualizar NOTA BANCO: " << notaInfo.xml_path;
    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "banco nao abriu atualizarNotaBanco";
            return false;
        }
    }
    QSqlQuery q;
    int idcliente;
    q.prepare("SELECT id FROM clientes WHERE cpf = :cnpjemit");
    q.bindValue(":cnpjemit", nf.cnpjemit);
    if(!q.exec()){
        qDebug() << "nao executou query para achar idcliente em atualizarNotaBanco()";
        return false;

    }else{
        if (q.next()) {
            idcliente = q.value(0).toInt();
        }
    }
    QString dhemi = notaInfo.dhEmi;
    QDateTime dt = QDateTime::fromString(dhemi, "dd/MM/yyyy HH:mm:ss");
    QString dhemiFormatada = dt.toString("yyyy-MM-dd HH:mm:ss");
    QDateTime dataIngles = QDateTime::currentDateTime();
    QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");
    q.prepare("UPDATE notas_fiscais SET "
              "cstat = :cstat, "
              "nnf = :nnf, "
              "serie = :serie, "
              "modelo = :modelo, "
              "tp_amb = :tp_amb, "
              "xml_path = :xml_path, "
              "valor_total = :valor_total, "
              "cnpjemit = :cnpjemit, "
              "nprot = :nprot, "
              "cuf = :cuf, "
              "atualizado_em = :atualizadoem, "
              "finalidade = :finalidade, "
              "saida = :saida, "
              "id_emissorcliente = :idcliente, "
              "dhemi = :dhemi "
              "WHERE chnfe = :chnfe");
    q.bindValue(":cstat", nf.cstat);
    q.bindValue(":nnf", nf.nnf);
    q.bindValue(":serie", nf.serie);
    q.bindValue(":modelo", nf.modelo);
    q.bindValue(":tp_amb", nf.tp_amb);
    q.bindValue(":xml_path", nf.xml_path);
    q.bindValue(":valor_total", nf.valor_total);
    q.bindValue(":cnpjemit", nf.cnpjemit);
    q.bindValue(":nprot", nf.nprot);
    q.bindValue(":cuf", nf.cuf);
    q.bindValue(":chnfe", nf.chnfe);
    q.bindValue(":atualizadoem", dataFormatada);
    q.bindValue(":finalidade", "ENTRADA EXTERNA");
    q.bindValue(":saida", "0");
    q.bindValue(":idcliente", idcliente);
    q.bindValue(":dhemi", dhemiFormatada);


    if (!q.exec()){
        qDebug() << "Erro ao atualizar NF:" << q.lastError();
        return false;
    }else{
        qDebug() << "Nota fiscal atualizada com sucesso!";
    }
    qDebug() << "Linhas afetadas:" << q.numRowsAffected();
    return true;
}

bool ManifestadorDFe::salvarEmitenteCliente(ProcNfe notaInfo){
    Emitente emi = lerEmitenteDoXML(notaInfo.xml_path);

    if(!db.open()){
        qDebug() << "banco de dados nao aberto salvarEmitenteCliente";
        return false;
    }
    bool ehPf = false;
    int indiedest = 1;
    if(emi.cnpj.length() == 14){
        ehPf =false;
    }else{
        ehPf = true;
    }
    if(!emi.ie.isEmpty()){
        indiedest = 1;
    }else{
        indiedest = 0;
    }


    QDateTime dataIngles = QDateTime::currentDateTime();
    QString dataFormatada = dataIngles.toString("yyyy-MM-dd HH:mm:ss");
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM clientes WHERE cpf = :cpf");
    check.bindValue(":cpf", emi.cnpj);

    if (!check.exec()) {
        qDebug() << "Erro ao verificar cliente existente:" << check.lastError();
        return false;
    }

    check.next();
    int total = check.value(0).toInt();

    if (total > 0) {
        qDebug() << "Cliente já cadastrado com este CNPJ/CPF!";
        return true;  // evita inserir duplicado
    }
    QSqlQuery query;
    query.prepare("INSERT INTO clientes (nome, email, telefone, endereco, cpf, "
                  "data_nascimento, data_cadastro, eh_pf, numero_end, bairro, "
                  "xMun, cMun, uf, cep, indIEDest, ie) VALUES (:nome, :email, :telefone, :endereco, :cpf, "
                  ":data_nascimento, :data_cadastro, :eh_pf, :numero_end, :bairro, "
                  ":xMun, :cMun, :uf, :cep, :indIEDest, :ie)");
    query.bindValue(":nome", emi.nome);
    query.bindValue(":email", "");
    query.bindValue(":telefone", "");
    query.bindValue(":endereco", emi.xLgr);
    query.bindValue(":cpf", emi.cnpj);
    query.bindValue(":data_nascimento", "");
    query.bindValue(":data_cadastro", dataFormatada);
    query.bindValue(":eh_pf", ehPf);
    query.bindValue(":numero_end", emi.nro);
    query.bindValue(":bairro", emi.xBairro);
    query.bindValue(":xMun", emi.xMun);
    query.bindValue(":cMun", emi.cMun);
    query.bindValue(":uf", emi.uf);
    query.bindValue(":cep", emi.cep);
    query.bindValue(":indIEDest", indiedest);
    query.bindValue(":ie", emi.ie);

    if(!query.exec()){
        qDebug() << "Query insert salvarEmitenteCliente nao funcionou!";
        return false;
    }else{
        qDebug() << "Fornecedor adicionado com sucesso!";

    }
    return true;

}

NotaFiscal ManifestadorDFe::lerNotaFiscalDoXML(const QString &xmlPath)
{
    NotaFiscal nf;
    nf.xml_path = xmlPath;

    QFile file(xmlPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Erro ao abrir XML:" << xmlPath;
        return nf;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Erro ao carregar XML:" << xmlPath;
        file.close();
        return nf;
    }
    file.close();

    auto getTag = [&](const QDomElement &parent, const QString &tag) {
        QDomNode n = parent.elementsByTagName(tag).item(0);
        return n.isNull() ? QString() : n.toElement().text().trimmed();
    };

    // Pegando nó nfeProc ou NFe
    QDomNodeList listaInfNFe = doc.elementsByTagName("infNFe");
    if (listaInfNFe.isEmpty()) {
        qWarning() << "XML sem <infNFe>";
        return nf;
    }

    QDomElement infNFe = listaInfNFe.at(0).toElement();

    // CHAVE DA NFE (atributo Id removendo "NFe")
    QString id = infNFe.attribute("Id");
    if (id.startsWith("NFe"))
        nf.chnfe = id.mid(3);

    // --- TAGS DIRETAS DO INFNFE ---
    QDomNodeList ideList = infNFe.elementsByTagName("ide");
    if (!ideList.isEmpty()) {
        QDomElement ide = ideList.at(0).toElement();
        nf.cuf   = getTag(ide, "cUF");
        nf.nnf   = getTag(ide, "nNF").toInt();
        nf.serie = getTag(ide, "serie");
        nf.modelo = getTag(ide, "mod");
        nf.tp_amb = (getTag(ide, "tpAmb") == "1");  // 1 = produção
    }

    // --- VALOR TOTAL DA NOTA ---
    QDomNodeList totalList = infNFe.elementsByTagName("ICMSTot");
    if (!totalList.isEmpty()) {
        QDomElement tot = totalList.at(0).toElement();
        nf.valor_total = getTag(tot, "vNF").toDouble();
    }

    // --- EMITENTE ---
    QDomNodeList emitList = infNFe.elementsByTagName("emit");
    if (!emitList.isEmpty()) {
        QDomElement emite = emitList.at(0).toElement();
        nf.cnpjemit = getTag(emite, "CNPJ");
    }

    // --- <protNFe> (protocolo) ---
    QDomNodeList protList = doc.elementsByTagName("protNFe");
    if (!protList.isEmpty()) {
        QDomElement prot = protList.at(0).toElement();
        QDomElement infProt = prot.elementsByTagName("infProt").item(0).toElement();

        nf.cstat = getTag(infProt, "cStat");
        nf.nprot = getTag(infProt, "nProt");
    }

    return nf;
}



bool ManifestadorDFe::enviarCienciaOperacao(const QString &chNFe, const QString &cnpjEmit)
{

    EventoCienciaOP *evento = new EventoCienciaOP(this, chNFe);


    // Envia
    EventoRetornoInfo info =  evento->gerarEnviarRetorno();
    salvarEventoNoBanco("Ciencia de Operacao", info, chNFe);
    qDebug() << "Manifestacao enviada:";

    bool sucesso = info.cStat == "128" || info.cStat == "135" || info.cStat == "136";
    return sucesso;
}

void ManifestadorDFe::salvarEventoNoBanco(const QString &tipo, const EventoRetornoInfo &info, const QString &chaveNFe)
{
    if (!db.open()) {
        qDebug() << "Erro ao abrir banco ao salvar evento.";
        return;
    }

    QSqlQuery q;
    QString idnf;
    q.prepare("SELECT id FROM notas_fiscais WHERE chnfe = :chnfe");
    q.bindValue(":chnfe", chaveNFe);
    if(!q.exec()){
        qDebug() << "nao executouy query para achar idnf no evneto ciencia";

    }else{
        if (q.next()) {
            idnf = q.value(0).toString();
        }
    }


    q.prepare(R"(
        INSERT INTO eventos_fiscais
        (tipo_evento, id_lote, cstat, justificativa, codigo, xml_path, nprot, id_nf)
        VALUES
        (:tipo, :lote, :cstat, :just, :codigo, :xml, :nprot,
            (SELECT id FROM notas_fiscais WHERE chnfe = :chave LIMIT 1)
        )
    )");

    q.bindValue(":tipo", tipo);
    q.bindValue(":lote", info.idLote);
    q.bindValue(":cstat", info.cStat);
    q.bindValue(":just", info.xMotivo);
    q.bindValue(":codigo", "210210");
    q.bindValue(":xml", info.xmlPath);
    q.bindValue(":nprot", info.nProt);
    q.bindValue(":chave", chaveNFe);
    q.bindValue(":id_nf", idnf);

    if (!q.exec())
        qDebug() << "Erro ao salvar evento_fiscal:" << q.lastError();

}

Emitente ManifestadorDFe::lerEmitenteDoXML(const QString &xmlPath) {
    Emitente e;
    QFile file(xmlPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Erro ao abrir XML:" << xmlPath;
        return e;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Erro ao ler XML:" << xmlPath;
        return e;
    }

    file.close();

    auto getTag = [&](const QDomElement &parent, const QString &tag) {
        QDomNode n = parent.elementsByTagName(tag).item(0);
        return n.isNull() ? QString() : n.toElement().text().trimmed();
    };

    // Posiciona no nó <emit>
    QDomNodeList emits = doc.elementsByTagName("emit");
    if (emits.isEmpty()) {
        qWarning() << "XML sem <emit>";
        return e;
    }

    QDomElement emitEl = emits.at(0).toElement();

    // Dados diretos
    e.cnpj = getTag(emitEl, "CNPJ");
    e.nome = getTag(emitEl, "xNome");
    e.ie   = getTag(emitEl, "IE");

    // Endereço
    QDomNode endNode = emitEl.elementsByTagName("enderEmit").item(0);
    if (!endNode.isNull()) {
        QDomElement end = endNode.toElement();
        e.xLgr   = getTag(end, "xLgr");
        e.nro    = getTag(end, "nro");
        e.xBairro= getTag(end, "xBairro");
        e.xMun   = getTag(end, "xMun");
        e.cMun   = getTag(end, "cMun");
        e.uf     = getTag(end, "UF");
        e.cep    = getTag(end, "CEP");
    }

    return e;
}

bool ManifestadorDFe::possoConsultar(){
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "Banco não abriu em possoConsultar";
            return false;
        }
    }

    QSqlQuery query;

    // Pega a MAIOR data_modificado (a mais recente), considerando xml e resumo
    if (!query.exec("SELECT MAX(data_modificado) FROM dfe_info")) {
        qDebug() << "Erro ao consultar dfe_info:" << query.lastError();
        return false;
    }

    if (!query.next()) {
        qDebug() << "Nenhum registro encontrado em dfe_info";
        return true; // nunca consultou → pode consultar
    }

    QString dataModStr = query.value(0).toString();

    if (dataModStr.isEmpty()) {
        return true; // sem data registrada
    }

    QDateTime dataMod = QDateTime::fromString(dataModStr, "yyyy-MM-dd HH:mm:ss");

    if (!dataMod.isValid()) {
        qDebug() << "Data inválida em dfe_info:" << dataModStr;
        return true; // se inválida, permite consultar
    }

    QDateTime agora = QDateTime::currentDateTime();

    qint64 diff = dataMod.secsTo(agora);

    qDebug() << "Diferença em segundos desde última consulta:" << diff;

    // 1 hora = 3600s
    return diff >= 3600;
}
