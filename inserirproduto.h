#ifndef INSERIRPRODUTO_H
#define INSERIRPRODUTO_H

#include <QWidget>
#include <QSqlDatabase>
#include <QLocale>
#include "util/ibptutil.h"


namespace Ui {
class InserirProduto;
}

class InserirProduto : public QWidget
{
    Q_OBJECT

public:
    explicit InserirProduto(QWidget *parent = nullptr);
    ~InserirProduto();


private slots:
    void on_Btn_GerarCBarras_clicked();

    void on_Ledit_CBarras_returnPressed();
    void on_Btn_Enviar_clicked();

    void on_Ledit_PrecoFornecedor_textChanged(const QString &arg1);

    void on_Ledit_PercentualLucro_textChanged(const QString &arg1);

    void on_Ledit_PrecoFinal_textChanged(const QString &arg1);

    void on_Ledit_NCM_editingFinished();

    void on_Ledit_Desc_editingFinished();

private:

    Ui::InserirProduto *ui;
    QMap<QString, QString> financeiroValues;
    QSet<QString> generatedNumbers;
    QLocale portugues;
    QString gerarNumero();
    bool verificarCodigoBarras();
    QSqlDatabase db = QSqlDatabase::database();
    bool atualizando = false; // Flag para evitar loops recursivos
    //bool eventFilter(QObject *watched, QEvent *event) override;
    IbptUtil *util;

signals:
    void codigoBarrasExistenteSignal(QString &query);
    void produtoInserido();
};

#endif // INSERIRPRODUTO_H
