#include "contingencia_service.h"
#include "../infra/apppath_service.h"
#include <QFile>
#include <QMutexLocker>
#include <QDebug>
#include <QSqlError>

static const int     INTERVALO_MS  = 5 * 60 * 1000;   // 5 minutos
static const QString DB_CONN_NAME  = QStringLiteral("contingencia_worker");

// ================================================================
// ContingenciaWorker
// ================================================================

ContingenciaWorker::ContingenciaWorker(QMutex *acbrMutex, QObject *parent)
    : QObject{parent}
    , m_timer(new QTimer(this))
    , m_nfe(AcbrManager::instance()->nfe())
    , m_mutex(acbrMutex)
{
    connect(m_timer, &QTimer::timeout, this, &ContingenciaWorker::tentarReenviar);
}

void ContingenciaWorker::setup()
{
    // Cria conexão SQLite exclusiva para este thread
    m_db = QSqlDatabase::addDatabase("QSQLITE", DB_CONN_NAME);
    m_db.setDatabaseName(AppPath_service::databasePath());
    if (!m_db.open()) {
        qDebug() << "ContingenciaWorker: falha ao abrir banco:" << m_db.lastError().text();
        return;
    }

    m_repo = new ContingenciaRepository(m_db, this);

    m_timer->start(INTERVALO_MS);
    qDebug() << "ContingenciaWorker: iniciado. Intervalo:" << INTERVALO_MS / 1000 << "s";
}

// ---- helpers ACBr (todas as chamadas protegidas pelo mutex) -----

static void parsearRetornoAcbr(const QString &retorno,
                                QString &cstat, QString &nProt, QString &xMotivo)
{
    for (const QString &linha : retorno.split('\n', Qt::SkipEmptyParts)) {
        QString l = linha.trimmed();
        if (l.startsWith("CStat="))
            cstat = l.section('=', 1).trimmed();
        else if (l.startsWith("NProt=") || l.startsWith("nProt="))
            nProt = l.section('=', 1).trimmed();
        else if (l.startsWith("XMotivo=") || l.startsWith("xMotivo="))
            xMotivo = l.section('=', 1).trimmed();
    }
}

bool ContingenciaWorker::consultarEAtualizar(const QString &chNfe)
{
    try {
        QString retConsulta;
        {
            QMutexLocker lock(m_mutex);
            retConsulta = QString::fromStdString(m_nfe->Consultar(chNfe.toStdString()));
        }

        QString cstat, nProt, xMotivo;
        parsearRetornoAcbr(retConsulta, cstat, nProt, xMotivo);

        qDebug() << "ContingenciaWorker: Consultar() para" << chNfe
                 << "→ cStat:" << cstat << xMotivo;

        if (cstat == "100" || cstat == "150") {
            m_repo->atualizar(chNfe, cstat, nProt);
            qDebug() << "ContingenciaWorker: nota já estava autorizada na SEFAZ —" << chNfe;
            return true;
        }
    } catch (std::exception &e) {
        qDebug() << "ContingenciaWorker: Consultar() falhou para" << chNfe << ":" << e.what();
    }
    return false;
}

void ContingenciaWorker::tentarReenviar()
{
    if (!m_repo) return;
    QList<NotaFiscalDTO> pendentes = m_repo->buscarPendentes();
    if (pendentes.isEmpty())
        return;

    qDebug() << "ContingenciaWorker: tentando reenviar" << pendentes.size() << "nota(s) em contingência";

    // Verifica disponibilidade da SEFAZ antes de tentar
    try {
        QString status;
        {
            QMutexLocker lock(m_mutex);
            status = QString::fromStdString(m_nfe->StatusServico());
        }
        bool disponivel = status.contains("CStat=107") || status.contains("CStat=108");
        if (!disponivel) {
            qDebug() << "ContingenciaWorker: SEFAZ indisponível, aguardando próximo ciclo";
            return;
        }
    } catch (...) {
        qDebug() << "ContingenciaWorker: sem conexão com SEFAZ";
        return;
    }

    for (const NotaFiscalDTO &nota : pendentes) {
        if (nota.xmlPath.isEmpty() || !QFile::exists(nota.xmlPath)) {
            qDebug() << "ContingenciaWorker: XML não encontrado para chave" << nota.chNfe << "- ignorando";
            continue;
        }

        try {
            QString ret;
            {
                QMutexLocker lock(m_mutex);
                m_nfe->LimparLista();
                m_nfe->ConfigGravarValor("NFe", "ModeloDF", nota.modelo == "55" ? "0" : "1");
                m_nfe->ConfigGravarValor("NFe", "FormaEmissao", "0");
                m_nfe->CarregarXML(nota.xmlPath.toStdString());
                ret = QString::fromUtf8(m_nfe->Enviar(1, false, true, false).c_str());
            }

            QString cstat, nProt, xMotivo;
            parsearRetornoAcbr(ret, cstat, nProt, xMotivo);

            qDebug() << "ContingenciaWorker: Enviar() →" << nota.chNfe
                     << "cStat:" << cstat << "|" << xMotivo;

            if (cstat == "100" || cstat == "150") {
                m_repo->atualizar(nota.chNfe, cstat, nProt);
                qDebug() << "ContingenciaWorker: nota retransmitida com sucesso —" << nota.chNfe;
            } else {
                // Pode já ter sido autorizada (timeout, duplicidade, etc.)
                if (!consultarEAtualizar(nota.chNfe)) {
                    // Rejeitada na SEFAZ — não tenta mais
                    m_repo->atualizar(nota.chNfe, "CONTINGENCIA_FALHA", "");
                    qDebug() << "ContingenciaWorker: nota marcada como falha definitiva —" << nota.chNfe;
                }
            }

        } catch (std::exception &e) {
            qDebug() << "ContingenciaWorker: erro ao retransmitir" << nota.chNfe << ":" << e.what();
            break; // Provável perda de conexão — aguarda próximo ciclo
        }
    }
}

// ================================================================
// ContingenciaService
// ================================================================

QMutex ContingenciaService::s_acbrMutex;

ContingenciaService::ContingenciaService(QObject *parent)
    : QObject{parent}
    , m_thread(new QThread(this))
    , m_worker(new ContingenciaWorker(&s_acbrMutex))
{
    m_worker->moveToThread(m_thread);

    // setup() roda no thread correto (após start)
    connect(m_thread, &QThread::started,  m_worker, &ContingenciaWorker::setup);
    // destrói o worker junto com o thread
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
}

ContingenciaService::~ContingenciaService()
{
    m_thread->quit();
    m_thread->wait();
}

void ContingenciaService::iniciar()
{
    m_thread->start();
    qDebug() << "ContingenciaService: thread de contingência iniciado";
}

QMutex *ContingenciaService::acbrMutex()
{
    return &s_acbrMutex;
}
