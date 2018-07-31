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
    return app.exec();
}
