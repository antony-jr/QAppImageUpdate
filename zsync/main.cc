#include <AppImageUpdateInformation>

using namespace AppImageUpdaterBridge;

int main(int ac, char **av)
{
    QCoreApplication app(ac , av);
    if(ac == 1){
	    return -1;
    }    
    AppImageUpdateInformation UpdateInformation;
    QString path(av[1]);

    QObject::connect(&UpdateInformation , &AppImageUpdateInformation::info , [&](QJsonObject information)
    {
    	qDebug() << "APPIMAGE PATH:: " << UpdateInformation.getAppImagePath();
    	qDebug() << "INFO:: " << information;
    	app.quit();
    	return;
    });

    UpdateInformation.setShowLog(true)
	    	     .setAppImage(path)
		     .getInfo();
    
    return app.exec();
}
