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
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QLocale portugues;

private slots:
    void on_Btn_Enviar_clicked();

    void on_Btn_Delete_clicked();

    void on_Btn_Pesquisa_clicked();

    void on_Btn_Alterar_clicked();

    void on_Btn_Venda_clicked();

    void on_Btn_Relatorios_clicked();
    
    void on_Ledit_Barras_returnPressed();

    void on_actionGerar_Relat_rio_PDF_triggered();

    void on_actionGerar_Relat_rio_CSV_triggered();

    void on_actionRealizar_Venda_triggered();

    void on_Btn_AddProd_clicked();

    QString gerarNumero();

    void on_Tview_Produtos_customContextMenuRequested(const QPoint &pos);

    void on_Btn_GerarCodBarras_clicked();

private:
    Ui::MainWindow *ui;
    bool verificarCodigoBarras();
    QSet<QString> generatedNumbers;
    QAction* actionMenuAlterarProd;
    QAction* actionMenuDeletarProd;

    QModelIndex selected_index;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;





};
#endif // MAINWINDOW_H
