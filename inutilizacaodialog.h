#ifndef INUTILIZACAODIALOG_H
#define INUTILIZACAODIALOG_H

#include <QDialog>

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
};

#endif // INUTILIZACAODIALOG_H
