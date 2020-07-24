#include <QCoreApplication>
#include <QCommandLineParser>
#include <QAppImageUpdate>
#include <cutelog.h>
#include <strings.h>

int main(int ac, char **av) {
    qInfo().noquote() << "SimpleUpdate, A Simple Updater using QAppImageUpdate.";
    qInfo().noquote() << "Copyright (C) 2018 , Antony Jr.";

    QCoreApplication app(ac, av);
    QAppImageUpdate updater;
    cutelog_t log = cutelog_new();


    QCommandLineParser parser;
    parser.process(app);
    auto args = parser.positionalArguments();
    if(args.count() == 0) {
        qInfo().noquote() << "\nUsage: " << app.arguments().at(0) << " [APPIMAGE PATH].";
        cutelog_free(log);
        return -1;
    }
    int it = 0;

    qInfo().noquote() << "";

    QObject::connect(&updater, &QAppImageUpdate::progress,
    [&](short operation, int percent, qint64 br, qint64 bt, double speed, QString unit) {
        Q_UNUSED(br);
        Q_UNUSED(bt);
        cutelog_mode(log, cutelog_non_multiline_mode);
        cutelog_progress(log, "[%i%% Done] Revising new version at %f %s.", percent, speed, unit.toStdString().c_str());
        return;
    });

    QObject::connect(&updater, &QAppImageUpdate::error, [&](short operation, short ecode) {
        qCritical().noquote() << "error:: " << QAppImageUpdate::errorCodeToString(ecode);
        cutelog_free(log);
        app.quit();
        return;
    });


    QObject::connect(&updater, &QAppImageUpdate::logger, [&](QString msg, QString path) {
        Q_UNUSED(path);
        cutelog_mode(log, cutelog_multiline_mode);

        if(msg.contains(QString::fromUtf8("INFO"))) {
            cutelog_info(log, "%s", msg.toStdString().c_str() + 9);
        } else if(msg.contains(QString::fromUtf8("WARNING"))) {
            cutelog_warning(log, "%s", msg.toStdString().c_str() + 9);
        } else if(msg.contains(QString::fromUtf8("FATAL"))) {
            cutelog_fatal(log, "%s", msg.toStdString().c_str() + 9);
        }
        return;
    });

    QObject::connect(&updater, &QAppImageUpdate::finished, [&](QJsonObject newVersion, QString oldAppImagePath) {
        cutelog_mode(log, cutelog_multiline_mode);
        cutelog_success(log, "Updated %s.", oldAppImagePath.toStdString().c_str());
        cutelog_success(log, "New Version: %s", ((newVersion["AbsolutePath"]).toString()).toStdString().c_str());

        ++it;
        if(it >= parser.positionalArguments().count()) {
            app.quit();
        } else {
            QString path(args[it]);
            updater.setAppImage(path);
            updater.start(confirm=true);
        }
        return;
    });

    QString path(args[it]);
    updater.setAppImage(path);

    updater.start(QAppImageUpdate::Operations::Update);
    return app.exec();
}
