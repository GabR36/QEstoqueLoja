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
    // // }
    // qDebug() << retorno;

    auto acbr = AcbrManager::instance()->nfe();
    QString ultimo_nsu = "892";

    std::string retorno = acbr->DistribuicaoDFePorUltNSU(cuf.toInt(), cnpj.toStdString(), ultimo_nsu.toStdString());
    QString whole = QString::fromStdString(retorno);

    QStringList blocos = whole.split("[", Qt::SkipEmptyParts);

    for (QString bloco : blocos)
    {
        bloco = "[" + bloco;

        if (bloco.startsWith("[ResNFe") || bloco.startsWith("[ResDFe"))
            processarResumo(bloco);
    }
}


void ManifestadorDFe::processarResumo(const QString &bloco)
{
    auto campo = [&](QString nome) {
        QRegularExpression re(nome + R"(=([^\r\n]*))");
        QRegularExpressionMatch m = re.match(bloco);
        return m.hasMatch() ? m.captured(1).trimmed() : "";
    };

    QString chave   = campo("chDFe");
    QString nome    = campo("xNome");
    // QString emissao = campo("dhEmi");
    QString cnpjEmit = campo("CNPJCPF");
    QString schema  = campo("schema");

    // Apenas notas completas ou resumos válidos
    if (!schema.contains("resNFe"))
        return;

    qDebug() << "Resumo localizado:" << chave << nome;

    enviarCienciaOperacao(chave, cnpjEmit);
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
    db = QSqlDatabase::database();
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

