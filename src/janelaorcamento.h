#ifndef JANELAORCAMENTO_H
#define JANELAORCAMENTO_H

#include <QWidget>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QLocale>
#include <QMenu>
#include <QAction>
#include "services/config_service.h"
#include "services/cliente_service.h"
#include "services/Produto_service.h"

namespace Ui {
class JanelaOrcamento;
}

class JanelaOrcamento : public QWidget
{
    Q_OBJECT

public:
    explicit JanelaOrcamento(QWidget *parent = nullptr);
    ~JanelaOrcamento();

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

    Cliente_service clienteService;
    Produto_Service produtoService;

    void configurarOrcamentoEstoque();
    void atualizarListaCliente();
    int validarCliente(bool mostrarMensagens);
    void selecionarClienteNovo();
    QString totalGeral();
    void atualizarTotalProduto();
};

#endif // JANELAORCAMENTO_H
