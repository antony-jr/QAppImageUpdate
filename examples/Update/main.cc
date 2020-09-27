#include <QCoreApplication>
#include <QCommandLineParser>
#include <QAppImageUpdate>

int main(int ac, char **av) {
    qInfo().noquote() << "Update, Update AppImages.";
    qInfo().noquote() << "Copyright (C) 2020, Antony Jr.";

    QCoreApplication app(ac, av);
    QAppImageUpdate updater;

    QCommandLineParser parser;
    parser.process(app);
    auto args = parser.positionalArguments();
    if(args.count() == 0) {
        qInfo().noquote() << "\nUsage: " << app.arguments().at(0) << " [APPIMAGE PATH].";
        return -1;
    }
    int it = 0;

    QObject::connect(&updater, &QAppImageUpdate::error, [&](short ecode, short action) {
        qCritical().noquote() << "error:: " << QAppImageUpdate::errorCodeToString(ecode);
        app.quit();
        return;
    });

    QObject::connect(&updater, &QAppImageUpdate::progress,
    [&](int percentage, qint64 rc, qint64 total, double speed, QString units) {
        qInfo().noquote() << "Updating " << percentage << "%: Revised " << rc << "/" << total
                          << " bytes at " << speed << units << "... ";
    });

    QObject::connect(&updater, &QAppImageUpdate::finished, [&](QJsonObject info, short action) {
        qInfo().noquote() << info;

        ++it;
        if(it >= parser.positionalArguments().count()) {
            app.quit();
        } else {
            QString path(args[it]);
            updater.setAppImage(path);
            updater.start(QAppImageUpdate::Action::Update);
        }
        return;
    });

    QString path(args[it]);
    updater.setAppImage(path);
    updater.setShowLog(true);
    updater.start(QAppImageUpdate::Action::Update);
    return app.exec();
}
