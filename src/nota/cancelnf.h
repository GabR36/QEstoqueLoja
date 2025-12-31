#ifndef CANCELNF_H
#define CANCELNF_H

#include <QObject>
#include "acbrmanager.h"
#include <QSqlDatabase>
#include <sstream>


class cancelNf : public QObject
{
    Q_OBJECT
public:
    explicit cancelNf(QObject *parent = nullptr, int idnf = 0);
    QString gerarEnviar();
    QString getPath();
private:
    ACBrNFe *acbr;
    std::stringstream ini;
    QSqlDatabase db;
    QString cnpjemit, chnfe, nprot, cuf;


    void pegarDados(int idnf);
    void preencherEvento();
signals:
};

#endif // CANCELNF_H
