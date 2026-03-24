#ifndef INUTILIZACAODIALOG_H
#define INUTILIZACAODIALOG_H

#include <QDialog>
#include "services/config_service.h"

namespace Ui {
class InutilizacaoDialog;
}

class InutilizacaoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InutilizacaoDialog(QWidget *parent = nullptr);
    ~InutilizacaoDialog();

private slots:
    void on_Btn_Enviar_clicked();

private:
    Ui::InutilizacaoDialog *ui;
    Config_service configServ;
    ConfigDTO configDTO;
};

#endif // INUTILIZACAODIALOG_H
