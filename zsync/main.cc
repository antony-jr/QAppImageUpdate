#include <AppImageDeltaWriter>
#include "block_downloader.hpp"

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    
    if(argc == 1){
	    return -1;
    }

    QString path(argv[1]);
    AppImageDeltaWriter Writer(path);
    BlockDownloader *downloader = new BlockDownloader(&Writer);

    QObject::connect(&Writer , &AppImageDeltaWriter::finished , [&](bool constructed){
	if(constructed){
		qDebug() << "Still need some blocks to complete the target file.";
		downloader->start();
	}else{
		qDebug() << "Constructed Target File , no need to download blocks.";
	qDebug() << "Exiting...";
	app.quit();
	}
	return;
    });

    Writer.setShowLog(true)
	  .start();
    return app.exec();
}
