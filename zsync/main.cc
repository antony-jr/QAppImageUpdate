#include <AppImageDeltaWriter>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    
    if(argc == 1){
	    return -1;
    }

    QString path(argv[1]);
    AppImageDeltaWriter Writer(path);

    QObject::connect(&Writer , &AppImageDeltaWriter::finished , [&](bool constructed){
	if(constructed){
		qDebug() << "Still need some blocks to complete the target file.";
		qDebug() << "BlockDownloader yet to implement.";
	}else{
		qDebug() << "Constructed Target File , no need to download blocks.";
	}
	qDebug() << "Exiting...";
	app.quit();
	return;
    });

    Writer.setShowLog(true)
	  .start();
    return app.exec();
}
