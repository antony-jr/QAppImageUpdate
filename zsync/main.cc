#include <AppImageUpdateInformation_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncWriter_p.hpp>
#include "block_downloader.hpp"

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    
    if(argc == 1){
	    return -1;
    }
    QString path(argv[1]);
    QNetworkAccessManager nm;
    AppImageUpdateInformationPrivate ui;
    ZsyncRemoteControlFileParserPrivate cp(&nm);
    ZsyncWriterPrivate w;
    BlockDownloader downloader(&w , &cp , &nm);

    QObject::connect(&ui , SIGNAL(info(QJsonObject)) , &cp , SLOT(setControlFileUrl(QJsonObject)));
    QObject::connect(&cp , &ZsyncRemoteControlFileParserPrivate::zsyncInformation ,
		     &w , &ZsyncWriterPrivate::setConfiguration);
    QObject::connect(&w , &ZsyncWriterPrivate::finishedConfiguring , &w , &ZsyncWriterPrivate::start);
    QObject::connect(&cp , &ZsyncRemoteControlFileParserPrivate::receiveControlFile , 
		     &cp , &ZsyncRemoteControlFileParserPrivate::getZsyncInformation);
    QObject::connect(&w , &ZsyncWriterPrivate::finished , [&](bool isDownloadNeeded)
    {
    if(isDownloadNeeded){
    qDebug() << "Downloading file.";
    downloader.start();
    }else{
    qDebug() << "File Constructed.";
    app.quit();
    }
    });
    QObject::connect(&w , &ZsyncWriterPrivate::error , [&](short code){
		    qDebug() << ZsyncWriterPrivate::errorCodeToString(code);
		    app.quit();
		    return;
    });
    cp.setShowLog(true);    
    ui.setShowLog(true);
    w.setShowLog(true);
    ui.setAppImage(path); 
    ui.getInfo();
    return app.exec();
}
