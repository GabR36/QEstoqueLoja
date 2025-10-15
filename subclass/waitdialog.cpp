#include "waitdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QCloseEvent>

WaitDialog::WaitDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Janela de espera");
    setModal(true);
    setWindowModality(Qt::ApplicationModal);
    setMinimumSize(400, 100);
   //setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);


    QVBoxLayout* layout = new QVBoxLayout(this);
    label = new QLabel("Carregando, por favor aguarde...");
    label->setWordWrap(true);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    labelErro = new QLabel("");
    labelErro->setWordWrap(true);
    labelErro->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    layout->addWidget(label);
    layout->addWidget(labelErro);
}

void WaitDialog::allowClose() {
    canClose = true;
}

void WaitDialog::setMessage(const QString& message) {
    label->setText(message);
}
void WaitDialog::setMessageErro(const QString &message){
    labelErro->setText(message);
}

void WaitDialog::closeEvent(QCloseEvent* event) {
    if (canClose) {
        event->accept(); // permite o fechamento
    } else {
        event->ignore(); // bloqueia Alt+F4, etc.
    }
}
