#include <AppImageDeltaWriter>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if(argc == 1){
	    return -1;
    }

    QString path(argv[1]);
    AppImageDeltaWriter dwriter(path);
    QObject::connect(&dwriter , &AppImageDeltaWriter::updateAvailable , [&](bool check, QString appimage){
	 qDebug().noquote() << QFileInfo(appimage).fileName() << ":: Update Available(" << check << ").";
	 app.quit();
	 return;
    });

    dwriter.setShowLog(true).checkForUpdate();
    return app.exec();
}
