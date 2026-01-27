#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "AkkoDriverHub_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-Bold.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-Demibold.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-ExtraLight.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-Heavy.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-Light.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-Medium.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-Normal.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-Regular.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-Semibold.ttf");
    QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/fonts/MiSans-Thin.ttf");

    a.setStyleSheet(R"(

        * { font-family: MiSans,MiSans;}
        QLabel#labelTitle1,#labelTitle2,#labelTitle3,#labelTitle4,#labelTitle5,#labelTitle6,#labelTitle,#labelName{ font-size: 16px; font-weight: 600; }
        QLabel#labelTitleL1,#labelTitleL2,#labelTitleL3,#labelTitleL4,#labelTitleL5 { font-size: 14px; font-weight: 500;}
        QLabel#labelTitleS1,#labelTitleS2,#labelTitleS3,#labelTitleS4,#labelTitleS5 { font-size: 16px; font-weight: 500;}
        QLabel#labelValue1,#labelValue5{ font-size: 10px; font-weight: 500;}
        QLabel#labelValue2,#labelValue4{ font-size: 14px; font-weight: 500;}
        QLabel#labelValue3,#labelValue6{ font-size: 18px; font-weight: 600;}
        QLabel#labelLarge,#labelLarge1,#labelLarge2{ font-size: 30px; font-weight: 600;}

)");

    MainWindow w;
    w.show();
    return a.exec();
}
