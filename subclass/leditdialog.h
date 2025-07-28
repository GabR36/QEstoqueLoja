#ifndef LEDITDIALOG_H
#define LEDITDIALOG_H

#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class LeditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LeditDialog(QWidget* parent = nullptr);

    void setLabelText(const QString& text);
    void setLineEditText(const QString& text);
    QString getLineEditText() const;

    void setCompleterSuggestions(const QStringList &suggestions);
    QLabel *Lbl_info;
    QLineEdit *Ledit_info;
    QPushButton* Btn_ok;
    QPushButton* Btn_cancel;
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private:


};

#endif // LEDITDIALOG_H
