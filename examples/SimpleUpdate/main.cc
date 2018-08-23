#include <QCoreApplication>
#include <AppImageUpdaterBridge>
#include <TextProgressBar.hpp>

using namespace AppImageUpdaterBridge;

int main(int ac, char **av)
{
    qInfo().noquote() << "SimpleUpdate , A Simple Updater using AppImageUpdaterBridge.";
    qInfo().noquote() << "Copyright (C) 2018 , Antony Jr.";

    if(ac == 1) {
        qInfo().noquote() << "\nUsage: " << av[0] << " [APPIMAGE PATH].";
        return -1;
    }
    QCoreApplication app(ac, av);
    QString path(av[1]);
    AppImageDeltaRevisioner DRevisioner(path);
    TextProgressBar progressBar;

    QObject::connect(&DRevisioner, &AppImageDeltaRevisioner::progress,
    [&](int percent, qint64 br, qint64 bt, double speed, QString unit) {
        progressBar.setStatus(percent, br, bt);
        progressBar.setMessage(QString::fromLatin1("Revising New Version at %1 %2").arg(speed, 3, 'f', 1).arg(unit));
        progressBar.update();
        return;
    });

    QObject::connect(&DRevisioner, &AppImageDeltaRevisioner::error, [&](short ecode) {
        qCritical().noquote() << "error:: " << AppImageDeltaRevisioner::errorCodeToString(ecode);
        if(ecode == AppImageDeltaRevisioner::UNKNOWN_NETWORK_ERROR) {
            qCritical().noquote() << "Network error:: " << DRevisioner.getNetworkError();
        }
        app.quit();
        return;
    });

    QObject::connect(&DRevisioner, &AppImageDeltaRevisioner::finished, [&]() {
        qInfo() << "\nCompleted Delta Update!";
        app.quit();
        return;
    });

    DRevisioner.start();
    return app.exec();
}
