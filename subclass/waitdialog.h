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

protected:
    void closeEvent(QCloseEvent *event) override;
private:
    QLabel* label;
    bool canClose = false;

};

#endif // WAITDIALOG_H
