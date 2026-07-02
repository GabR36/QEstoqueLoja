#include "export_service.h"
#include "../infra/apppath_service.h"
#include <QFile>

Export_service::Export_service(QObject *parent)
    : QObject{parent}
{}

Export_service::Resultado Export_service::exportarSqliteDB(QString pathDestino){

    if (pathDestino.isEmpty())
        return {false, "Caminho vazio."};

    QString origem;
    origem = AppPath_service::databasePath();

    // Nome do arquivo exportado
    QString destino = pathDestino + "/estoque_backup.db";

    // Remove backup antigo se existir
    if (QFile::exists(destino))
        QFile::remove(destino);

    if (QFile::copy(origem, destino))
    {
        return {true, "Banco SQLite exportado com sucesso!"};
    }else{
        return{false, "Não foi possível exportar o banco.\n\nOrigem:\n" + origem};
    }
}
