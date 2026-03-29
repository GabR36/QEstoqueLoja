#ifndef INFOJANELAPROD_H
#define INFOJANELAPROD_H

#include "alterarproduto.h"
#include "../services/Produto_service.h"

class InfoJanelaProd : public AlterarProduto
{
    Q_OBJECT

public:
    explicit InfoJanelaProd(QWidget *parent = nullptr, QString id = "1");

private:
    Produto_Service infoService;
};

#endif // INFOJANELAPROD_H
