#ifndef MERGEPRODUTOS_H
#define MERGEPRODUTOS_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSqlDatabase>

namespace Ui {
class MergeProdutos;
}

class MergeProdutos : public QWidget
{
    Q_OBJECT

public:
    explicit MergeProdutos(QVariantMap produto1, QVariantMap produto2, QVariantMap sugerido, QWidget *parent = nullptr);
    ~MergeProdutos();

private slots:
    void on_Btn_Cancelar_clicked();

    void on_Btn_Importar_clicked();

private:
    Ui::MergeProdutos *ui;
    QStandardItemModel *modeloComparacaoProd;
    QSqlDatabase db;
    QString idProduto;
    bool nfProduto;
};

#endif // MERGEPRODUTOS_H
