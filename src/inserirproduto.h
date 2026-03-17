#ifndef INSERIRPRODUTO_H
#define INSERIRPRODUTO_H

#include <QWidget>
#include <QSqlDatabase>
#include <QLocale>
#include "util/ibptutil.h"
#include "services/Produto_service.h"
#include "services/config_service.h"
#include "dto/Produto_dto.h"


namespace Ui {
class InserirProduto;
}

class InserirProduto : public QWidget
{
    Q_OBJECT

public:
    explicit InserirProduto(QWidget *parent = nullptr);
    ~InserirProduto();
    void preencherCamposProduto(const ProdutoDTO &prod);
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
    QSet<QString> generatedNumbers;
    QLocale portugues;
    QSqlDatabase db = QSqlDatabase::database();
    bool atualizando = false; // Flag para evitar loops recursivos
    //bool eventFilter(QObject *watched, QEvent *event) override;
    IbptUtil *util;
    Produto_Service service;
    ConfigDTO configDTO;

    void carregarConfiguracoes();
signals:
    void codigoBarrasExistenteSignal(QString &codigo);
    void produtoInserido();
};

#endif // INSERIRPRODUTO_H
