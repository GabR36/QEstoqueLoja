#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if defined(Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


#include <QMainWindow>
#include <QStandardItemModel>
#include <vector>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QLocale>
#include <QSet>
#include <QRandomGenerator>
#include <QKeyEvent>
#include "configuracao.h"
#include "subclass/customlineedit.h"
#include "nota/acbrmanager.h"
#include "../services/Produto_service.h"
#include "dto/Config_dto.h"


#define VERSAO_QE "2.4.0"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:


    QSqlDatabase db;
    QSqlQueryModel* model = nullptr;
    void atualizarTableview();
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QLocale portugues;
    QIcon iconAlterarProduto, iconAddProduto, iconBtnVenda, iconDelete, iconPesquisa, iconBtnRelatorios,
        iconImpressora, iconClientes;

public slots:
   void atualizarTableviewComQuery(QString &query);

private slots:

    void on_Btn_Delete_clicked();

    void on_Btn_Pesquisa_clicked();

    void on_Btn_Alterar_clicked();

    void on_Btn_Venda_clicked();

    void on_Btn_Relatorios_clicked();

    void on_actionRealizar_Venda_triggered();

    void on_Btn_AddProd_clicked();

    void on_Tview_Produtos_customContextMenuRequested(const QPoint &pos);

    void imprimirEtiqueta1();
    void imprimirEtiqueta3();

    void on_actionConfig_triggered();

    void on_Ledit_Pesquisa_textChanged(const QString &arg1);

    void on_Btn_Clientes_clicked();

    void setLocalProd();

    void verProd();


    void on_actionDocumenta_o_triggered();
    void atualizarConfigAcbr();

    void on_Btn_Entradas_clicked();

    void on_actionEnviar_triggered();

    void on_actionSobre_triggered();

    void on_actionMonitor_Fiscal_triggered();

private:
    Ui::MainWindow *ui;
    // ACBrNFe *acbr;
    bool verificarCodigoBarras();
    QSet<QString> generatedNumbers;
    QAction* actionMenuAlterarProd;
    QAction* actionMenuDeletarProd;
    QAction* actionSetLocalProd;
    QAction* actionMenuPrintBarCode1;
    QAction* actionMenuPrintBarCode3;
    QAction* actionVerProduto;
    Produto_Service *produtoService;
    ConfigDTO configDTO;


    void setarIconesJanela();
    //QModelIndex selected_index;

    const int ultimaVersaoSchema = 7;


    void mostrarProdutoPorCodigoBarras(const QString &codigo);
    void iniciarMigration();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    QString getIdProdSelected();
signals:
    void localSetado();





};
#endif // MAINWINDOW_H
