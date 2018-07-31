#include <AppImageUpdateInformation>
#include <AppImageInspector>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    if(argc == 1) {
        qInfo().noquote() << "Usage  : " << argv[0] << " [OPTIONS] [FILES ...]";
        return 0;
    }

    QCoreApplication app(argc, argv);
    AppImageUpdateInformation info;
    QStringList files;

    /* print copyright. */
    qInfo().noquote() << "UpdateInformation v3.0.0 , Multi-Threaded AppImage Information extractor.";
    qInfo().noquote() << "Copyright BSD-3 Clause Licence Antony J.r.";

    argv++;
    while(*argv) {
        files.append(*argv);
        ++argv;
    }

    qInfo() << "files  :: " << files;
    
    info.setAppImage(files.at(0));

    QFutureWatcher<void> watcher;

    QObject::connect(&watcher , &QFutureWatcher<void>::finished , [&]()
    {
    qDebug() << "Everything Finished.";
    app.quit();
    return;
    });

    QFuture<void> future = QtConcurrent::map(files, [&](QString AppImagePath) { 
	QEventLoop loop;
	AppImageInspector *inspector = new AppImageInspector(&info);
	QObject::connect(inspector , &AppImageInspector::updatesAvailable , &loop , &QEventLoop::quit);
	QObject::connect(inspector , &AppImageInspector::updatesNotAvailable , &loop , &QEventLoop::quit);
	inspector->checkForUpdates();
	loop.exec();

	qDebug() << "AppImageUpdaterBridge(" << info.getAppImagePath() << "):: Update Needed(" <<
		 inspector->isUpdatesAvailable() << ").";
	delete inspector;
	return;
    });

    watcher.setFuture(future);

    return app.exec();
}
