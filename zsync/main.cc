#include <QCoreApplication>
#include "textprogressbar.hpp"
#include <AppImageUpdaterBridge>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    
    if(argc == 1){
	    return -1;
    }
    QString path(argv[1]);
    AppImageDeltaRevisioner r(path);
    TextProgressBar progressBar;
    
    QObject::connect(&r , &AppImageDeltaRevisioner::started , [&]()
    {
    qInfo().noquote() << "AppImageUpdaterBridge v0.0.1 , Simple AppImage Updater in Qt.";
    qInfo().noquote() << "Copyright (C) 2018 , Antony J.r.\n";
    return;
    });

    QObject::connect(&r , &AppImageDeltaRevisioner::progress , [&](int percent , qint64 br , qint64 bt , double speed , QString unit)
    {
    progressBar.setStatus(percent , br , bt);
    progressBar.setMessage(QString::fromLatin1("Revising New Version at %1 %2").arg(speed, 3, 'f', 1).arg(unit));
    progressBar.update();
    return;
    });

    QObject::connect(&r , &AppImageDeltaRevisioner::finished , [&]()
    {
    qInfo() << "\nFinished Successfully.";
    app.quit();
    return;
    });
    r.start();
    return app.exec();
}
