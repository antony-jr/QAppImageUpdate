#include <AppImageDeltaWriter.hpp>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if(argc == 1){
	    return -1;
    }

    QString path(argv[1]);
    AppImageUpdateResource res(path);
    res.setShowLog(true);
    AppImageDeltaWriter dwriter(&res);

    QObject::connect(&dwriter , &AppImageDeltaWriter::updateAvailable , [&](bool check, QString appimage){
	 qDebug().noquote() << QFileInfo(appimage).fileName() << ":: Update Available(" << check << ").";
	 app.quit();
	 return;
    });

    dwriter.checkForUpdate();
    return app.exec();
}
