#include "leditdialog.h"
#include <QCompleter>
#include <QEvent>

LeditDialog::LeditDialog(QWidget* parent) : QDialog(parent) {
    // Inicializa os widgets
    Lbl_info = new QLabel("Digite algo:", this);
    Ledit_info = new QLineEdit(this);
    Btn_ok = new QPushButton("OK", this);
    Btn_cancel = new QPushButton("Cancelar", this);

    // Conecta botões aos slots padrão
    connect(Btn_ok, &QPushButton::clicked, this, &QDialog::accept);
    connect(Btn_cancel, &QPushButton::clicked, this, &QDialog::reject);

    // Layout dos botões
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(Btn_ok);
    buttonLayout->addWidget(Btn_cancel);

    // Layout principal
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(Lbl_info);
    mainLayout->addWidget(Ledit_info);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    Ledit_info->installEventFilter(this);
    setWindowTitle("Entrada de Texto");
    setFixedSize(300, 120);
}
void LeditDialog::setLabelText(const QString& text)
{
    Lbl_info->setText(text);
}

void LeditDialog::setLineEditText(const QString& text)
{
    Ledit_info->setText(text);
}

QString LeditDialog::getLineEditText() const
{
    return Ledit_info->text();
}
void LeditDialog::setCompleterSuggestions(const QStringList& suggestions)
{
    auto* completer = new QCompleter(suggestions, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);  // Ignore maiúsculas/minúsculas
    completer->setFilterMode(Qt::MatchContains);         // Sugestões que contêm o texto

    Ledit_info->setCompleter(completer);
    connect(Ledit_info, &QLineEdit::textEdited, this, [this]() {
        if (Ledit_info->completer()) {
            Ledit_info->completer()->complete(); // Força a abrir a lista
        }
    });
}
bool LeditDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == Ledit_info && event->type() == QEvent::MouseButtonPress) {
        if (Ledit_info->completer()) {
            Ledit_info->completer()->complete(); // Abre o popup
        }
    }
    return QDialog::eventFilter(obj, event); // chama comportamento padrão
}

