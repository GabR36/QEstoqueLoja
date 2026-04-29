#ifndef CARTACORRECAOJANELA_H
#define CARTACORRECAOJANELA_H

#include <QDialog>
#include "services/eventofiscal_service.h"


namespace Ui {
class CartaCorrecaoJanela;
}

class CartaCorrecaoJanela : public QDialog
{
    Q_OBJECT

public:
    explicit CartaCorrecaoJanela(QWidget *parent = nullptr);
    ~CartaCorrecaoJanela();

private slots:
    void on_Btn_Enviar_clicked();

private:
    Ui::CartaCorrecaoJanela *ui;
    EventoFiscal_service eventoServ;
};

#endif // CARTACORRECAOJANELA_H
