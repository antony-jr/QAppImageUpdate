#include <AppImageUpdateInformation>
#include <AppImageInspector>
#include <QtConcurrent>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    if(argc == 1) {
        qInfo().noquote() << "Usage  : " << argv[0] << " [OPTIONS] [FILES ...]";
        return 0;
    }

    QCoreApplication app(argc, argv);
    QNetworkAccessManager qnam;
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
    
    QtConcurrent::map(files, [&](QString AppImagePath) { 
	AppImageUpdateInformation *updateInformation = new AppImageUpdateInformation;
	updateInformation->setShowLog(true).setAppImage(AppImagePath);
	AppImageInspector *inspector = new AppImageInspector(updateInformation , &qnam);
	inspector->setShowLog(true).checkForUpdates();
	return;
    });
    return app.exec();
}
