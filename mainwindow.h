#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString nomeProduto, quantidadeProduto, registro, descProduto;
    int rowCount;
    QStandardItemModel *model = new QStandardItemModel();
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_Btn_Enviar_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
