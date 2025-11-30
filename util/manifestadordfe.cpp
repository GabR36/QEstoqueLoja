#include "manifestadordfe.h"
#include <QRegularExpression>
#include "../nota/acbrmanager.h"
#include "../nota/eventocienciaop.h"
#include <QSqlQuery>
#include <QDateTime>
#include <QDebug>
#include "../configuracao.h"
#include <QFile>

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
void ManifestadorDFe::processarHeaderDfe(const QString &bloco){
    auto campo = [&](QString nome) {
        QRegularExpression re(nome + R"(=([^\r\n]*))");
        QRegularExpressionMatch m = re.match(bloco);
        return m.hasMatch() ? m.captured(1) : QString();
    };

    QString ultNsu = campo("ultNSU");

    // 1) remover sequências LITERAIS "\n" e "\r" (backslash + letra)
    ultNsu.replace("\\n", "");   // remove duas chars '\' 'n'
    ultNsu.replace("\\r", "");   // remove '\' 'r'

    // 2) remover quebras de linha reais
    ultNsu.remove('\n');
    ultNsu.remove('\r');

    // 3) trim final e começo
    ultNsu = ultNsu.trimmed();

    qDebug() << "ultNSU da consulta (normalizado):" << ultNsu;
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

    // enviarCienciaOperacao(resumo.chave, resumo.cnpjEmit);
    salvarResumoNota(resumo);
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

    if (!q.exec())
        qDebug() << "Erro ao salvar evento_fiscal:" << q.lastError();


}

