#ifndef CONTINGENCIA_SERVICE_H
#define CONTINGENCIA_SERVICE_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QSqlDatabase>
#include "../dto/NotaFiscal_dto.h"
#include "../nota/acbrmanager.h"
#include "../repository/contingencia_repository.h"

// ----------------------------------------------------------------
// ContingenciaWorker – executa inteiramente no thread dedicado
// ----------------------------------------------------------------
class ContingenciaWorker : public QObject
{
    Q_OBJECT
public:
    explicit ContingenciaWorker(QMutex *acbrMutex, QObject *parent = nullptr);

public slots:
    /** Chamado pelo QThread::started – abre conexão DB e inicia o timer. */
    void setup();
    /** Disparado pelo timer a cada intervalo. */
    void tentarReenviar();

private:
    bool consultarEAtualizar(const QString &chNfe);

    QTimer                *m_timer;
    QSqlDatabase           m_db;
    ContingenciaRepository *m_repo = nullptr;
    ACBrNFe               *m_nfe;
    QMutex                *m_mutex; // não possui — aponta para ContingenciaService::s_acbrMutex
};

// ----------------------------------------------------------------
// ContingenciaService – ponto de entrada público
// ----------------------------------------------------------------
class ContingenciaService : public QObject
{
    Q_OBJECT
public:
    explicit ContingenciaService(QObject *parent = nullptr);
    ~ContingenciaService();

    void iniciar();

    /**
     * Mutex que protege todas as chamadas ao ACBrNFe.
     * Outros serviços que usam nfe() devem bloquear este mutex
     * enquanto o ContingenciaService estiver ativo.
     */
    static QMutex *acbrMutex();

private:
    static QMutex      s_acbrMutex;
    QThread           *m_thread;
    ContingenciaWorker *m_worker;
};

#endif // CONTINGENCIA_SERVICE_H
