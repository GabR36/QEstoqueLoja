#include "contingencia_service.h"
#include <QFile>
#include <QDebug>

static const int INTERVALO_MS = 5 * 60 * 1000; // 5 minutos

ContingenciaService::ContingenciaService(QObject *parent)
    : QObject{parent}
{
    nfe   = AcbrManager::instance()->nfe();
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ContingenciaService::tentarReenviar);
}

void ContingenciaService::iniciar()
{
    timer->start(INTERVALO_MS);
    qDebug() << "ContingenciaService iniciado. Intervalo:" << INTERVALO_MS / 1000 << "s";
}

// Parseia cStat, xMotivo e NProt de uma resposta INI do ACBr
static void parsearRetornoAcbr(const QString &retorno, QString &cstat, QString &nProt, QString &xMotivo)
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

// Consulta o status real da nota na SEFAZ e atualiza o banco se autorizada
bool ContingenciaService::consultarEAtualizar(const QString &chNfe)
{
    try {
        QString retConsulta = QString::fromStdString(nfe->Consultar(chNfe.toStdString()));
        QString cstat, nProt, xMotivo;
        parsearRetornoAcbr(retConsulta, cstat, nProt, xMotivo);

        qDebug() << "ContingenciaService: Consultar() para" << chNfe
                 << "→ cStat:" << cstat << xMotivo;

        if (cstat == "100" || cstat == "150") {
            repo.atualizarRetornoContingencia(chNfe, cstat, nProt);
            qDebug() << "ContingenciaService: nota já estava autorizada na SEFAZ —" << chNfe;
            return true;
        }
    } catch (std::exception &e) {
        qDebug() << "ContingenciaService: Consultar() falhou para" << chNfe << ":" << e.what();
    }
    return false;
}

void ContingenciaService::tentarReenviar()
{
    QList<NotaFiscalDTO> pendentes = repo.buscarContingencias();
    if (pendentes.isEmpty())
        return;

    qDebug() << "ContingenciaService: tentando reenviar" << pendentes.size() << "nota(s) em contingência";

    // Verifica disponibilidade da SEFAZ antes de tentar
    try {
        QString status = QString::fromStdString(nfe->StatusServico());
        bool disponivel = status.contains("CStat=107") || status.contains("CStat=108");
        if (!disponivel) {
            qDebug() << "ContingenciaService: SEFAZ indisponível, aguardando próximo ciclo";
            return;
        }
    } catch (...) {
        qDebug() << "ContingenciaService: sem conexão com SEFAZ";
        return;
    }

    for (const NotaFiscalDTO &nota : pendentes) {
        if (nota.xmlPath.isEmpty() || !QFile::exists(nota.xmlPath)) {
            qDebug() << "ContingenciaService: XML não encontrado para chave" << nota.chNfe << "- ignorando";
            continue;
        }

        try {
            nfe->LimparLista();
            nfe->ConfigGravarValor("NFe", "ModeloDF", nota.modelo == "55" ? "0" : "1");
            nfe->ConfigGravarValor("NFe", "FormaEmissao", "0");
            nfe->CarregarXML(nota.xmlPath.toStdString());

            std::string retorno = nfe->Enviar(1, false, true, false);
            QString ret = QString::fromUtf8(retorno.c_str());

            QString cstat, nProt, xMotivo;
            parsearRetornoAcbr(ret, cstat, nProt, xMotivo);

            qDebug() << "ContingenciaService: Enviar() →" << nota.chNfe
                     << "cStat:" << cstat << "|" << xMotivo;

            if (cstat == "100" || cstat == "150") {
                repo.atualizarRetornoContingencia(nota.chNfe, cstat, nProt);
                qDebug() << "ContingenciaService: nota retransmitida com sucesso —" << nota.chNfe;
            } else {
                // Nota pode já ter sido autorizada anteriormente (timeout, duplicidade, etc.)
                // Consulta o status real diretamente na SEFAZ
                consultarEAtualizar(nota.chNfe);
            }

        } catch (std::exception &e) {
            qDebug() << "ContingenciaService: erro ao retransmitir" << nota.chNfe << ":" << e.what();
            break; // Provável perda de conexão — aguarda próximo ciclo
        }
    }
}
