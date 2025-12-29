#ifndef INFOJANELAPROD_H
#define INFOJANELAPROD_H

#include <QDialog>
#include <QSqlDatabase>
#include <QLocale>

namespace Ui {
class InfoJanelaProd;
}

class InfoJanelaProd : public QDialog
{
    Q_OBJECT

public:
    explicit InfoJanelaProd(QWidget *parent = nullptr, int id = 1);
    ~InfoJanelaProd();

private:
    Ui::InfoJanelaProd *ui;
    QSqlDatabase db = QSqlDatabase::database();
    QLocale portugues;
    QString quant,desc,precoFinal,codigoBarras,ucom,precoForn,porcentLucro,
        ncm,cest,aliquotaIcms,csosn,pis,local;
    bool nf;


};

#endif // INFOJANELAPROD_H
