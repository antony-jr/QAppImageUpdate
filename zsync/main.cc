#include <ZsyncRemoteControlFileParser_p.hpp>
#include <AppImageUpdateInformation_p.hpp>
#include <ZsyncWriter_p.hpp>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    
    if(argc == 1){
	    return -1;
    }

    QString path(argv[1]);
    QNetworkAccessManager qnam;
    AppImageUpdateInformationPrivate Upd;
    Upd.setAppImage(path);
    Upd.setShowLog(true);
    ZsyncRemoteControlFileParserPrivate cfp(&qnam);
    cfp.setShowLog(true);
    ZsyncWriterPrivate Writer;

    QObject::connect(&Upd , SIGNAL(info(QJsonObject)) , &cfp , SLOT(setControlFileUrl(QJsonObject)));
    QObject::connect(&cfp , &ZsyncRemoteControlFileParserPrivate::zsyncInformation , 
		    &Writer , &ZsyncWriterPrivate::setConfiguration);
    QObject::connect(&Writer, &ZsyncWriterPrivate::finishedConfiguring , &Writer , &ZsyncWriterPrivate::start);
    QObject::connect(&Writer, &ZsyncWriterPrivate::finished, [&](bool isDownloadNeeded){
		    if(isDownloadNeeded){
		    qDebug() << "Download Needed";
		    }
		    app.quit();
		    return;
    });
    QObject::connect(&cfp , SIGNAL(receiveControlFile(void)) , &cfp , SLOT(getZsyncInformation(void)));

    Upd.getInfo();

    return app.exec();
}
