#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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


//#include "vendas.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:


    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QSqlQueryModel* model = new QSqlQueryModel;
    void atualizarTableview();
    void imprimirEtiqueta(int quant, QString codBar = "1", QString desc = "null",  QString preco = "");
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QLocale portugues;
    QIcon iconAlterarProduto, iconAddProduto, iconBtnVenda, iconDelete, iconPesquisa, iconBtnRelatorios,
        iconImpressora, iconClientes;
    QString gerarNumero();
    static QString normalizeText(const QString &text);


public slots:
   void on_actionTodos_Produtos_triggered();

   void atualizarTableviewComQuery(QString &query);

private slots:
    void on_Btn_Enviar_clicked();

    void on_Btn_Delete_clicked();

    void on_Btn_Pesquisa_clicked();

    void on_Btn_Alterar_clicked();

    void on_Btn_Venda_clicked();

    void on_Btn_Relatorios_clicked();
    
    void on_Ledit_Barras_returnPressed();

    void on_actionGerar_Relat_rio_CSV_triggered();

    void on_actionRealizar_Venda_triggered();

    void on_Btn_AddProd_clicked();

    void on_Tview_Produtos_customContextMenuRequested(const QPoint &pos);

    void on_Btn_GerarCodBarras_clicked();

    void imprimirEtiqueta1();
    void imprimirEtiqueta3();


    void on_actionApenas_NF_triggered();

    void on_actionConfig_triggered();

    void on_Ledit_Pesquisa_textChanged(const QString &arg1);

    void on_Btn_Clientes_clicked();

private:
    Ui::MainWindow *ui;
    bool verificarCodigoBarras();
    QSet<QString> generatedNumbers;
    QAction* actionMenuAlterarProd;
    QAction* actionMenuDeletarProd;
    QAction* actionMenuPrintBarCode1;
    QAction* actionMenuPrintBarCode3;
    QMap<QString, QString> financeiroValues;


    void setarIconesJanela();
    //QModelIndex selected_index;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;





};
#endif // MAINWINDOW_H
