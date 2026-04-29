#ifndef EVENTOFISCAL_SERVICE_H
#define EVENTOFISCAL_SERVICE_H

#include <QObject>
#include <QSqlQueryModel>
#include <QMap>
#include <QList>
#include <QPair>
#include <QDateTime>
#include "../repository/eventofiscal_repository.h"
#include "../dto/EventoFiscal_dto.h"
#include "../nota/cancelnf.h"
#include "../services/notafiscal_service.h"

enum class EventoFiscalErro{
    Nenhum,
    Banco,
    InsercaoInvalida,
    UpdateInvalido,
    EnvioEvento,
    EventoRecusadoSefaz,
    Desconhecido,
    QuebraDeRegra
};


class EventoFiscal_service : public QObject
{
    Q_OBJECT
public:
    struct Resultado {
        bool ok;
        EventoFiscalErro erro = EventoFiscalErro::Nenhum;
        QString msg;
    };
    explicit EventoFiscal_service(QObject *parent = nullptr);
    EventoFiscal_service::Resultado inserir(EventoFiscalDTO evento);
    EventoFiscal_service::Resultado enviarCancelamento(qlonglong idnf);
    void listarTodos(QSqlQueryModel *model);
    QMap<QString, int>             contarPorTipo(QDateTime dtIni, QDateTime dtFim);
    QList<QPair<QString, QString>> buscarXmlsPorPeriodo(QDateTime dtIni, QDateTime dtFim);
    EventoFiscal_service::Resultado enviarCienciaOp(QString chnfe, QString &retornoforcado);
    EventoFiscal_service::Resultado enviarCCE(QString chnfe = "", int nseq = 1, QString correcao = "", QString retornoforcado = "");
private:
    EventoFiscal_repository eventoRepo;
    NotaFiscal_service nfServ;

signals:
};

#endif // EVENTOFISCAL_SERVICE_H
