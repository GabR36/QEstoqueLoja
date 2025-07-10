#ifndef PRODUTOTABLEVIEW_H
#define PRODUTOTABLEVIEW_H

#include <QTableView>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QDebug>

class ProdutoTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ProdutoTableView(QWidget *parent = nullptr);
    ~ProdutoTableView();

    bool isPromoted() const { return true; } // Método para testar promoção
    QSqlQueryModel* model;

    QSqlQueryModel *getModel();
private:
    QSqlDatabase db;

    void configurar();
    int getIdProdSelected();
    void verProd();
};

#endif // PRODUTOTABLEVIEW_H
