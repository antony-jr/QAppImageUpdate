#include <QApplication>
#include <QCommandLineParser>
#include <AppImageUpdaterBridge>

using namespace AppImageUpdaterBridge;

int main(int ac, char **av)
{
    qInfo().noquote() << "SimpleUpdate , A Simple Updater using AppImageUpdaterBridge.";
    qInfo().noquote() << "Copyright (C) 2018 , Antony Jr.";

    QApplication app(ac, av);
    QCommandLineParser parser;
    parser.process(app);
    auto args = parser.positionalArguments();
    if(args.count() == 0) {
        qInfo().noquote() << "\nUsage: " << app.arguments().at(0) << " [APPIMAGE PATH].";
        return -1;
    }
    AppImageUpdaterDialog UWidget(args[0]); 
    QObject::connect(&UWidget, &AppImageUpdaterDialog::error , [&](QString eStr, short errorCode){
        qInfo() << "error code:: " << errorCode;
        return;
    });

    QObject::connect(&UWidget , &AppImageUpdaterDialog::quit , &app , &QApplication::quit , Qt::QueuedConnection);
    QObject::connect(&UWidget , &AppImageUpdaterDialog::canceled , &app , &QApplication::quit , Qt::QueuedConnection);

    QObject::connect(&UWidget, &AppImageUpdaterDialog::finished, [&](QJsonObject newVersion) {
        (void)newVersion;
        app.quit();
	return;
    });
    UWidget.init();
    return app.exec();
}
