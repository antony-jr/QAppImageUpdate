#include <AppImageUpdateInformation>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if(argc == 1){
	    return -1;
    }

    QString path(argv[1]);
    AppImageUpdateInformation Information(path);
    auto count = 100;
    QTimer timer,
	   timer2;
    timer.setSingleShot(true);
    timer.setInterval(5);
    timer2.setSingleShot(true);
    timer2.setInterval(10);

    QObject::connect(&timer , &QTimer::timeout , [&]()
    {
    timer.stop();
    timer.setInterval(5);
    timer.start();
    qDebug() << "Its Still Non Blocking.";
    Information.getInfo();
    return;
    });

    QObject::connect(&timer2 , &QTimer::timeout , [&]()
    {
    timer2.stop();
    timer2.setInterval(10);
    timer2.start();
    qDebug() << "More Stress.";
    Information.getInfo();
    return;
    });

    QObject::connect(&Information , &AppImageUpdateInformation::info , [&](QJsonObject info)
    {
    qDebug() << "INFO:: " << info;
    if(!count){
    timer.stop();
    timer2.stop();
    app.quit();
    }else{
    --count;
    }
    return;
    });

    Information.getInfo();
    timer.start();
    timer2.start();
    qDebug() << "Non Blocking.";
    return app.exec();
}
