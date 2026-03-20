#ifndef JANELAORCAMENTO_H
#define JANELAORCAMENTO_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QLocale>
#include <QMenu>
#include <QAction>
#include "services/config_service.h"

namespace Ui {
class JanelaOrcamento;
}

class JanelaOrcamento : public QWidget
{
    Q_OBJECT

public:
    explicit JanelaOrcamento(QWidget *parent = nullptr);
    ~JanelaOrcamento();

    QSqlDatabase db = QSqlDatabase::database();

private slots:
    void on_Btn_AddProd_clicked();
    void on_Ledit_PesquisaProduto_textChanged(const QString &arg1);
    void on_Btn_Terminar_clicked();
    void on_Btn_NovoCliente_clicked();
    void on_Tview_ProdutosSelec_customContextMenuRequested(const QPoint &pos);
    void deletarProd();

private:
    Ui::JanelaOrcamento *ui;

    QLocale portugues;
    QStandardItemModel *modeloSelecionados = new QStandardItemModel;
    QStringList clientesComId;
    QAction *actionMenuDeletarProd;
    ConfigDTO configDTO;

    void configurarOrcamentoEstoque();
    void atualizarListaCliente();
    int validarCliente(bool mostrarMensagens);
    QPair<QString, int> extrairNomeId(const QString &texto);
    bool verificarNomeIdCliente(const QString &nome, int id);
    void selecionarClienteNovo();
    QString totalGeral();
    void atualizarTotalProduto();
};

#endif // JANELAORCAMENTO_H
