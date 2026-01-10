#ifndef SOBRE_H
#define SOBRE_H

#include <QDialog>

namespace Ui {
class Sobre;
}

class Sobre : public QDialog
{
    Q_OBJECT

public:
    explicit Sobre(QWidget *parent = nullptr);
    ~Sobre();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Sobre *ui;
};

#endif // SOBRE_H
