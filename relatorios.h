#ifndef RELATORIOS_H
#define RELATORIOS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QItemSelection>
#include "mainwindow.h"
#include <QPainter>
#include <QFileDialog>
#include <QPdfWriter>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QDesktopServices>
#include <QLocale>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>

namespace Ui {
class relatorios;
}

class relatorios : public QWidget
{
    Q_OBJECT

public:

    explicit relatorios(QWidget *parent = nullptr);
    QSqlDatabase db = QSqlDatabase::database();

    ~relatorios();

    QMap<QString, int> buscarVendasPorMes();

    QStringList buscarAnosDisponiveis();
    QMap<QString, int> buscarVendasPorMesAno(const QString &ano);
    QMap<QString, int> buscarQuantidadePorFormaPagamento();
    QMap<QString, QPair<double, double> > buscarValorVendasPorMesAno(const QString &ano);
    QMap<QString, QVector<int> > buscarFormasPagamentoPorAno(const QString &anoSelecionado);
protected:
    QMap<QString, int> buscarVendasPorDiaMesAno(const QString &ano, const QString &mes);
    QMap<QString, int> buscarTopProdutosVendidos();
    QMap<QString, double> buscarValorVendasPorDiaMesAno(const QString &ano, const QString &mes);
    QString totalGeral();
    void atualizarTotalProduto();
    bool existeProdutoVendido();
    QMap<QString,QString> fiscalValues;
    QMap<QString, float> produtosMaisLucrativosAno(const QString &ano);
private slots:
    void on_Btn_PdfGen_clicked();

    void on_Btn_CsvGen_clicked();

    void on_Btn_AddProd_clicked();

    void on_Ledit_PesquisaProduto_textChanged(const QString &arg1);

    void on_Btn_Terminar_clicked();

    void on_Btn_NovoCliente_clicked();

    void on_tabWidget_tabBarClicked(int index);

    void on_Tview_ProdutosSelec_customContextMenuRequested(const QPoint &pos);

    void deletarProd();


private:
    Ui::relatorios *ui;
    void conectarBancoDados();
    QLocale portugues;
    QStringList meses = {"01 - Janeiro", "02 - Fevereiro", "03 - Março", "04 - Abril", "05 - Maio",
                         "06 - Junho", "07 - Julho", "08 - Agosto", "09 - Setembro",
                         "10 - Outubro", "11 - Novembro", "12 - Dezembro"};
    QStandardItemModel *modeloSelecionados = new QStandardItemModel;
    QStringList clientesComId;
    QAction *actionMenuDeletarProd;


    void configurarJanelaQuantVendas();
    void configurarJanelaValorVendas();
    void configurarJanelaTopProdutosVendas();
    void configurarJanelaFormasPagamentoAno();
    void configurarOrcamentoEstoque();
    void atualizarListaCliente();
    int validarCliente(bool mostrarMensagens);
    QPair<QString, int> extrairNomeId(const QString &texto);
    bool verificarNomeIdCliente(const QString &nome, int id);
    void selecionarClienteNovo();
    void configurarJanelaNFValor();
    QMap<QString, float> buscarValoresNfAno(const QString &ano);
    void configurarJanelaProdutoLucroValor();
};

#endif // RELATORIOS_H
