#include <AppImageUpdateResource>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if(argc == 1){
	    return -1;
    }

    QString path(argv[1]);
    auto res = new AppImageUpdateResource(path , &app);
    QObject::connect(res , &AppImageUpdateResource::info , [&](QJsonObject infor)
    {
    qDebug() << "INFO:: " << infor;
    app.quit();
    return;
    });
    res->setShowLog(true).getInfo();
    return app.exec();
}
