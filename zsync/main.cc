#include <AppImageUpdateInformation>
#include <AppImageInspector>
#include <QtConcurrent>
#include <QFuture>

using namespace AppImageUpdaterBridge;

int main(int ac, char **av)
{
    QCoreApplication app(ac , av);
    if(ac == 1){
	    return -1;
    }    
    QNetworkAccessManager qnam;
    QString path(av[1]);
    QtConcurrent::run([&]()
    {
    	AppImageUpdateInformation *info = new AppImageUpdateInformation;
	info->setShowLog(true).setAppImage(path);
	AppImageInspector *ins = new AppImageInspector(info , &qnam);
	ins->setShowLog(true).checkForUpdates();
    });
    return app.exec();
}
