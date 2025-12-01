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
    QString ultNsuXml = getUltNsuXml();
    // std::string retorno = acbr->DistribuicaoDFePorUltNSU(cuf.toInt(), cnpj.toStdString(), ultNsuXml.toStdString());
    // qDebug() << "Retorno consulta DFE" << retorno;
    // qDebug() << "ult nsuxml: " << ultNsuXml;

    QFile file("retorno_distribuicao2.txt");
    QString retorno;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        retorno = QString::fromUtf8(file.readAll());
        file.close();
    } else {
        qDebug() << "Erro ao abrir retorno.txt";
    }

    QString whole = QString::fromStdString(retorno.toStdString());

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

    for (QString bloco : blocos)
    {
        bloco = "[" + bloco;

        if (bloco.startsWith("[ResNFe") || bloco.startsWith("[ResDFe"))
            processarNota(bloco);
    }
    // for (QString bloco : blocos)
    // {
    //     bloco = "[" + bloco;

    //     if (bloco.startsWith("[DistribuicaoDFe"))
    //         processarHeaderDfe(bloco);
    // }
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
        return;
    }

    // Somente sucesso > atualiza NSU
    salvarNovoUltNsu(ultNsu);
}


void ManifestadorDFe::salvarNovoUltNsu(const QString &ultNsu){
    if (!db.open()) {
        qDebug() << "Erro ao abrir DB em salvarNovoUltNsu:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE dfe_info SET ult_nsu = :nsu WHERE identificacao = 'consulta_resumo'");
    query.bindValue(":nsu", ultNsu);

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
    QSqlQuery query;
    query.prepare("INSERT INTO notas_fiscais (cstat, modelo, tp_amb, xml_path, valor_total, atualizado_em,"
                  "cnpjemit, chnfe, nprot, cuf, finalidade, saida, nnf, serie) VALUES (:cstat, :modelo,"
                  " :tpamb, :xmlpath, :vnf, :atualizadoem, :cnpjemit, :chnf, :nprot, :cuf, :finalidade, "
                  ":saida, :nnf, :serie)");
    query.bindValue(":cstat", resumo.cstat);
    query.bindValue(":modelo", "55");
    query.bindValue(":tpamb", fiscalValues.value("tp_amb"));
    query.bindValue(":xmlpath", resumo.xml_path);
    query.bindValue(":vnf", resumo.vnf);
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
        QRegularExpression re("^" + nome + R"(=(.*))", QRegularExpression::MultilineOption);
        QRegularExpressionMatch m = re.match(bloco);
        if (!m.hasMatch())
            return QString();

        QString value = m.captured(1);
        return value.trimmed();   // <-- remove qualquer \r, \n, espaços
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

    // Apenas notas completas ou resumos válidos
    if (!resumo.schema.contains("resNFe"))
        return;

    qDebug() << "\nResumo localizado:" << resumo.chave << resumo.nome;
    salvarResumoNota(resumo);
    enviarCienciaOperacao(resumo.chave, resumo.cnpjEmit);
}

void ManifestadorDFe::processarNota(const QString &bloco){
    auto campo = [&](const QString &nome) {
        QRegularExpression re("^" + nome + R"(=(.*))", QRegularExpression::MultilineOption);
        QRegularExpressionMatch m = re.match(bloco);
        if (!m.hasMatch())
            return QString();

        QString value = m.captured(1);
        return value.trimmed();   // <-- remove qualquer \r, \n, espaços
    };
        ProcNfe nfe;
        nfe.chave    = campo("chDFe");
        nfe.nome     = campo("xNome");
        nfe.cnpjEmit = campo("CNPJCPF");
        nfe.schema   = campo("schema");
        nfe.vnf      = campo("vNF");
        nfe.cstat    = campo("CStat");
        nfe.xml_path = campo("arquivo");
        nfe.nProt    = campo("nProt");
        nfe.dhEmi    = campo("dhEmi");
        nfe.nsu = campo("NSU");
        nfe.cSitNfe = campo("cSitNFe");

        // Apenas notas completas ou resumos válidos
        if (!nfe.schema.contains("procNFe"))
            return;

        qDebug() << "\nNota Xml localizado:" << nfe.chave << nfe.nome;
        salvarEmitenteCliente(nfe);
        // salvarResumoNota(nfe);
        // enviarCienciaOperacao(nfe.chave, nfe.cnpjEmit);
}

void ManifestadorDFe::salvarEmitenteCliente(ProcNfe notaInfo){
    Emitente emi = lerEmitenteDoXML(notaInfo.xml_path);

    if(!db.open()){
        qDebug() << "banco de dados nao aberto salvarEmitenteCliente";
        return;
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
        return;
    }

    check.next();
    int total = check.value(0).toInt();

    if (total > 0) {
        qDebug() << "Cliente já cadastrado com este CNPJ/CPF!";
        return;  // evita inserir duplicado
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
    }else{
        qDebug() << "Fornecedor adicionado com sucesso!";
    }

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
    q.bindValue(":codigo", tipo);
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
