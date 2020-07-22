#include <QApplication>
#include <QCommandLineParser>
#include <QAppImageUpdate>

int main(int ac, char **av) {
    qInfo().noquote() << "SimpleUpdate, A Simple Updater using QAppImageUpdate.";
    qInfo().noquote() << "Copyright (C) 2018, Antony Jr.";

    QApplication app(ac, av);
    QCommandLineParser parser;
    parser.process(app);
    int it = 0;
    auto args = parser.positionalArguments();
    if(args.count() == 0) {
        qInfo().noquote() << "\nUsage: " << app.arguments().at(0) << " [APPIMAGE PATH].";
        return -1;
    }

    QAppImageUpdate updater;

    QObject::connect(&updater, &QAppImageUpdate::error, [&](short errorCode) {
        Q_UNUSED(errorCode);
        qInfo() << "error:: "<< QAppImageUpdate::errorCodeToString(errorCode);
        app.quit();
        return;
    });

    QObject::connect(&updater, &QAppImageUpdate::quit, &app, &QApplication::quit, Qt::QueuedConnection);
    QObject::connect(&updater, &QAppImageUpdate::canceled, &app, &QApplication::quit, Qt::QueuedConnection);

    QObject::connect(&updater, &QAppImageUpdate::finished, [&](QJsonObject newVersion) {
        (void)newVersion;
        if(it < args.count()) {
            updater.setAppImage(args.at(it));
	    updater.start(gui=true, confirm=true);
            ++it;
        } else {
            app.quit();
        }
        return;
    });
    
    updater.setAppImage(args.at(it));
    updater.setShowLog(true);
    updater.start(gui=true, confirm=true);
    ++it;
    return app.exec();
}
