#ifndef ALTERARPRODUTO_H
#define ALTERARPRODUTO_H

#include <QDialog>

namespace Ui {
class AlterarProduto;
}

class AlterarProduto : public QDialog
{
    Q_OBJECT

public:
    explicit AlterarProduto(QWidget *parent = nullptr);
    ~AlterarProduto();
    void TrazerInfo(QString nome, QString desc, QString quant, QString preco);

private:
    Ui::AlterarProduto *ui;
};

#endif // ALTERARPRODUTO_H
