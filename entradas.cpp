#include "entradas.h"
#include "ui_entradas.h"
#include "nota/acbrmanager.h"
#include "configuracao.h"
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QFile>
#include <QDomDocument>
#include <QRegularExpression>

Entradas::Entradas(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Entradas)
{
    ui->setupUi(this);
    empresaValues = Configuracao::get_All_Empresa_Values();
    db = QSqlDatabase::database();
    carregarTabela();
}

Entradas::~Entradas()
{
    delete ui;
}

void Entradas::salvarRegistroDFe(
    const QString &nome_emitente,
    const QString &data_emissao,
    const QString &vnf,
    const QString &nsu,
    const QString &tipo,
    const QString &chave,
    const QString &cnpj,
    const QString &situacao,
    const QString &xml,
    const QString &data_recebimento)
{
    QSqlQuery query(db);

    query.prepare(R"(
        INSERT OR IGNORE INTO dfe_registros
        (nome_emitente, data_emissao, vnf, nsu, tipo, chave, cnpj, situacao, xml_path, data_recebimento)
        VALUES (:nome_emitente, :data_emissao, :vnf, :nsu, :tipo, :chave, :cnpj, :situacao, :xml_path, :data_recebimento)
    )");

    query.bindValue(":nome_emitente", nome_emitente);
    query.bindValue(":data_emissao", data_emissao);
    query.bindValue(":vnf", vnf);
    query.bindValue(":nsu", nsu);
    query.bindValue(":tipo", tipo);
    query.bindValue(":chave", chave);
    query.bindValue(":cnpj", cnpj);
    query.bindValue(":situacao", situacao);

    // Agora armazenamos xml (preferencialmente o path se houver)
    query.bindValue(":xml_path", xml);
    query.bindValue(":data_recebimento", data_recebimento);

    if (!query.exec()) {
        qDebug() << "Erro ao inserir registro:" << query.lastError().text();
    }
}

void Entradas::on_Btn_ConsultarDF_clicked()
{
    QString ultimo_nsu = "886";
    auto acbrnfe = AcbrManager::instance()->nfe();

    std::string retorno, cnpj;
    cnpj = empresaValues.value("cnpj_empresa").toStdString();
    retorno = acbrnfe->DistribuicaoDFePorUltNSU(41, cnpj, ultimo_nsu.toStdString());

    QString whole = QString::fromStdString(retorno);
    QString motivo;

    // 1) Tentar achar xMotivo
    QRegularExpression reMotivo(R"(xMotivo=([^\r\n]*))");
    auto m1 = reMotivo.match(whole);
    if (m1.hasMatch()) {
        motivo = m1.captured(1).trimmed();
    } else {
        // 2) Tentar Msg=
        QRegularExpression reMsg(R"(Msg=([^\r\n]*))");
        auto m2 = reMsg.match(whole);
        if (m2.hasMatch())
            motivo = m2.captured(1).trimmed();
        else
            motivo = "Não foi possível obter o motivo no retorno da SEFAZ!";
    }

    QMessageBox::information(this, "Retorno da SEFAZ", motivo);

    if (!db.open()) {
        qDebug() << "Não abriu BD";
        return;
    }

    // Função auxiliar: pega campo exatamente na linha "CHAVE=valor"
    auto getCampoLinha = [&](const QString &bloco, const QString &chave) {
        QRegularExpression re(QStringLiteral("(?m)^%1=([^\r\n]*)").arg(QRegularExpression::escape(chave)));
        auto m = re.match(bloco);
        if (m.hasMatch())
            return m.captured(1).trimmed();
        return QString();
    };

    // Função auxiliar: extrai <xNome> de um texto XML usando QDomDocument, com fallback por regex
    auto extrairXNomeDeXmlTexto = [&](const QString &xmlTexto) -> QString {
        if (xmlTexto.isEmpty()) return QString();

        // Tentar QDomDocument (mais robusto)
        QDomDocument doc;
        QString errorMsg;
        int errLine = 0, errCol = 0;
        if (doc.setContent(xmlTexto, &errorMsg, &errLine, &errCol)) {
            QDomNodeList nodes = doc.elementsByTagName("xNome");
            if (!nodes.isEmpty()) {
                QString t = nodes.at(0).toElement().text().trimmed();
                if (!t.isEmpty()) return t;
            }
            // às vezes o XML tem namespace default e elementsByTagName pode falhar.
        } else {
            qDebug() << "Aviso: falha ao parsear XML inline com QDom:" << errorMsg << "lin:" << errLine << "col:" << errCol;
        }

        // Fallback: regex para <xNome>...</xNome>
        QRegularExpression reXml("<xNome[^>]*>([^<]+)</xNome>", QRegularExpression::DotMatchesEverythingOption);
        auto m = reXml.match(xmlTexto);
        if (m.hasMatch())
            return m.captured(1).trimmed();

        return QString();
    };

    // Função auxiliar: extrai <xNome> a partir de um arquivo
    auto extrairXNomeDeArquivo = [&](const QString &path) -> QString {
        if (path.isEmpty()) return QString();
        QFile f(path);
        if (!f.exists()) {
            qDebug() << "Arquivo XML não encontrado:" << path;
            return QString();
        }
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Falha ao abrir XML:" << path << f.errorString();
            return QString();
        }
        QByteArray ba = f.readAll();
        f.close();
        return extrairXNomeDeXmlTexto(QString::fromUtf8(ba));
    };

    // Separar em blocos
    QStringList blocos = whole.split("[", Qt::SkipEmptyParts);

    for (QString bloco : blocos)
    {
        bloco = "[" + bloco;

        // Identificar tipo
        QString tipo;
        if (bloco.startsWith("[ResDFe"))
            tipo = "RESUMO";
        else if (bloco.startsWith("[ProEve"))
            tipo = "EVENTO";
        else if (bloco.startsWith("[InfEve"))
            tipo = "INFO_EVENTO";
        else
            continue;

        // Capturar campos por linha (mais seguro)
        QString nsu          = getCampoLinha(bloco, "NSU");
        QString nome         = getCampoLinha(bloco, "xNome");       // tentativa direta
        QString cnpj_emit    = getCampoLinha(bloco, "CNPJCPF");
        QString chave        = getCampoLinha(bloco, "chDFe");
        QString vnf          = getCampoLinha(bloco, "vNF");
        QString data_emissao = getCampoLinha(bloco, "dhEmi");
        QString data_receb   = getCampoLinha(bloco, "dhRecbto");
        QString situacao     = getCampoLinha(bloco, "cSitNFe");
        QString xmlInline    = getCampoLinha(bloco, "XML");        // XML inline (se houver)
        QString xmlpath      = getCampoLinha(bloco, "arquivo");    // caminho fornecido pelo ACBr
        QString schema       = getCampoLinha(bloco, "schema");

        //Filtra somente XML COMPLETO da nota
        if (!(schema.contains("NFe") || schema.contains("proc"))) {
            qDebug() << "Ignorando registro NSU" << nsu << " - não é XML completo.";
            continue;
        }

        // Se nome não foi encontrado diretamente, tentar extrair do XML inline
        if (nome.isEmpty() && !xmlInline.isEmpty()) {
            QString fromXml = extrairXNomeDeXmlTexto(xmlInline);
            if (!fromXml.isEmpty())
                nome = fromXml;
        }

        // Se ainda vazio, tentar ler do arquivo salvo pelo ACBr
        if (nome.isEmpty() && !xmlpath.isEmpty()) {
            QString fromFile = extrairXNomeDeArquivo(xmlpath);
            if (!fromFile.isEmpty())
                nome = fromFile;
        }

        // Fallback final: tentar pegar via regex dentro do próprio bloco (caso apareça <xNome> no bloco)
        if (nome.isEmpty()) {
            QRegularExpression reFallback("<xNome[^>]*>([^<]+)</xNome>", QRegularExpression::DotMatchesEverythingOption);
            auto mf = reFallback.match(bloco);
            if (mf.hasMatch())
                nome = mf.captured(1).trimmed();
        }

        // Se ainda vazio, marca como desconhecido (opcional)
        if (nome.isEmpty())
            nome = QStringLiteral("(NOME NÃO ENCONTRADO)");

        // Definir o valor a salvar no campo xml_path: prefira o path, senão o XML inline, senão vazio
        QString xmlParaSalvar;
        if (!xmlpath.isEmpty())
            xmlParaSalvar = xmlpath;
        else if (!xmlInline.isEmpty())
            xmlParaSalvar = xmlInline;
        else
            xmlParaSalvar = QString();

        // Salvar no banco
        salvarRegistroDFe(
            nome,
            data_emissao,
            vnf,
            nsu,
            tipo,
            chave,
            cnpj_emit,
            situacao,
            xmlParaSalvar,
            data_receb
            );
    }

    carregarTabela();
    qDebug() << "Processamento concluído!";
    qDebug() << retorno;
}

void Entradas::carregarTabela()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery("SELECT nome_emitente, data_emissao, vnf, nsu, tipo, "
                    "chave, cnpj, situacao, data_recebimento FROM dfe_registros "
                    "ORDER BY data_emissao DESC", db);

    model->setHeaderData(0, Qt::Horizontal, "Emitente");
    model->setHeaderData(1, Qt::Horizontal, "Emissão");
    model->setHeaderData(2, Qt::Horizontal, "Valor NF");
    model->setHeaderData(3, Qt::Horizontal, "NSU");
    model->setHeaderData(4, Qt::Horizontal, "Tipo");
    model->setHeaderData(5, Qt::Horizontal, "Chave");
    model->setHeaderData(6, Qt::Horizontal, "CNPJ");
    model->setHeaderData(7, Qt::Horizontal, "Situação");
    model->setHeaderData(8, Qt::Horizontal, "Recebido");

    ui->Tview_Entradas->setModel(model);
    ui->Tview_Entradas->resizeColumnsToContents();
    ui->Tview_Entradas->horizontalHeader()->setStretchLastSection(true);
}
