#include <AppImageUpdateInformation>
#include <AppImageInspector>

using namespace AppImageUpdaterBridge;

int main(int ac, char **av)
{
    QCoreApplication app(ac , av);
    if(ac == 1){
	    return -1;
    }    
    QNetworkAccessManager qnam;
    AppImageUpdateInformation UpdateInformation;
    AppImageInspector Inspector(&UpdateInformation , &qnam);
    QString path(av[1]);

    QObject::connect(&Inspector , &AppImageInspector::updatesAvailable , [&](bool isUpdatesAvailable)
    {
        qDebug() << "Update Available:: " << isUpdatesAvailable;
	app.quit();
        return;
    });

    UpdateInformation.setShowLog(true)
	    	     .setAppImage(path);

    Inspector.setShowLog(true).checkForUpdates();
    
    return app.exec();
}
