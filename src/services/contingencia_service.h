#ifndef CONTINGENCIA_SERVICE_H
#define CONTINGENCIA_SERVICE_H

#include <QObject>
#include <QTimer>
#include "../repository/notafiscal_repository.h"
#include "../nota/acbrmanager.h"

class ContingenciaService : public QObject
{
    Q_OBJECT
public:
    explicit ContingenciaService(QObject *parent = nullptr);
    void iniciar();

private slots:
    void tentarReenviar();

private:
    bool consultarEAtualizar(const QString &chNfe);

    QTimer *timer;
    notafiscal_repository repo;
    ACBrNFe *nfe;
};

#endif // CONTINGENCIA_SERVICE_H
