#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QLabel>

class WaitDialog : public QDialog
{
Q_OBJECT
public:
    explicit WaitDialog(QWidget* parent = nullptr);
    void setMessage(const QString& message);
    void allowClose();
    void setMessageErro(const QString &message);
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    QLabel* label;
    QLabel* labelErro;

    bool canClose = false;

};

#endif // WAITDIALOG_H
