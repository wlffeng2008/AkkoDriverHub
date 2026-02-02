#include "MainWindow.h"

#include <QApplication>
#include <QFontDatabase>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QTranslator translator;
    QTranslator translatorB;
    QTranslator translatorW;
    {
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "AkkoDriverHub_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
        QString baseName = QLocale::system().name(); // 如 "zh_CN"
        translatorB.load("qtbase_" + baseName, QLibraryInfo::path(QLibraryInfo::TranslationsPath));
        translatorW.load("qt_" + baseName, QLibraryInfo::path(QLibraryInfo::TranslationsPath));

        a.installTranslator(&translatorB);
        a.installTranslator(&translatorW);
    }


    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-Bold.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-Demibold.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-ExtraLight.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-Heavy.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-Light.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-Medium.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-Normal.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-Regular.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-Semibold.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
                                      "/fonts/MiSans-Thin.ttf");

    a.setStyleSheet(R"(

        * { font-family: MiSans,MiSans;}
        QLabel#labelTitle1,#labelTitle2,#labelTitle3,#labelTitle4,#labelTitle5,#labelTitle6,#labelTitle,#labelName{ font-size: 16px; font-weight: 600; }
        QLabel#labelTitleL1,#labelTitleL2,#labelTitleL3,#labelTitleL4,#labelTitleL5 { font-size: 14px; font-weight: 500;}
        QLabel#labelTitleS1,#labelTitleS2,#labelTitleS3,#labelTitleS4,#labelTitleS5 { font-size: 16px; font-weight: 500;}
        QLabel#labelValue1,#labelValue5{ font-size: 10px; font-weight: 500;}
        QLabel#labelValue2,#labelValue4{ font-size: 14px; font-weight: 500;}
        QLabel#labelValue3,#labelValue6{ font-size: 18px; font-weight: 600;}
        QLabel#labelLarge,#labelLarge1,#labelLarge2{ font-size: 40px; font-weight: 600;}
        QLabel#labelName,#labelName1,#labelName2{ font-size: 32px; font-weight: 600;}
        QLabel#labelInfo0{ font-size: 32px; font-weight: 600; color:#202020;}
        QScrollBar:vertical,
        QScrollBar:horizontal {
            background: transparent;
            border: none;
            margin: 0px;
            padding: 0px;
        }

        QScrollBar:vertical {
            width: 4px;
            margin: 0px 0px 0px 0px;
        }

        QScrollBar:horizontal {
            height: 4px;
            margin: 0px 2px 0px 2px;
        }

        QScrollBar::handle:vertical,
        QScrollBar::handle:horizontal {
            background: rgba(220, 220, 220, 0.3);
            border-radius: 2px;
            min-height: 20px;
            max-height: 20px;
            min-width: 20px;
        }

        QScrollBar::handle:vertical:hover,
        QScrollBar::handle:horizontal:hover { background: rgba(160, 250, 160, 0.7); }

        QScrollBar::handle:vertical:pressed,
        QScrollBar::handle:horizontal:pressed { background: rgba(160, 250, 160, 0.98); }

        QMessageBox QPushButton {
                border: 1px solid #6C9F50;
                background-color: #2D7FDD;
                color: white;
                border-radius: 8px;
                padding: 2px 2px;
                min-width: 80px;
                min-height: 24px; }

        QMessageBox {min-width: 500px; min-height: 250px;}
        QMessageBox QLabel#qt_msgbox_label{min-width: 450px; min-height: 120px; max-width: 450px; max-height: 520px; qproperty-alignment: AlignLeft; white-space: pre-wrap;font: bold 12px 微软雅黑;}
        QMessageBox QLabel#qt_msgboxex_icon_label{ min-width: 32px; min-height: 32px; max-width: 32px; max-height: 32px;qproperty-alignment: AlignTop;}


)");

    MainWindow w;
    w.show();
    return a.exec();
}
