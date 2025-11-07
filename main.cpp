    #include "mainwindow.h"

    #include <QApplication>
    #include <QLocale>
    #include <QTranslator>
    // #include <qguiapplication.h>
    #include <qicon.h>
    #include <QStyleFactory>

    int main(int argc, char *argv[])
    {

        // QGuiApplication app(argc, argv);
        // QGuiApplication::setWindowIcon(QIcon(":/QEstoqueLOja/QeLogo.png"));


        QApplication a(argc, argv);

        // Força estilo independente do sistema
        QApplication::setStyle(QStyleFactory::create("Fusion"));

        // Configura paleta clara manualmente
        QPalette lightPalette;

        lightPalette.setColor(QPalette::Window, QColor(255, 255, 255));
        lightPalette.setColor(QPalette::WindowText, Qt::black);
        lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
        lightPalette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::ToolTipBase, Qt::black);
        lightPalette.setColor(QPalette::ToolTipText, Qt::white);
        lightPalette.setColor(QPalette::Text, Qt::black);
        lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::ButtonText, Qt::black);
        lightPalette.setColor(QPalette::BrightText, Qt::red);
        lightPalette.setColor(QPalette::Link, QColor(0, 0, 255));

        lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
        lightPalette.setColor(QPalette::HighlightedText, Qt::white);

        a.setPalette(lightPalette);


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
