#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <qguiapplication.h>
#include <qicon.h>

int main(int argc, char *argv[])
{

    QGuiApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon(":/QEstoqueLOja/QeLogo.png"));


    QApplication a(argc, argv);


    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QEstoqueLoja_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
