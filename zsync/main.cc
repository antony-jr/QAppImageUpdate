#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncCore_p.hpp>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if(argc == 1){
	    return -2;
    }
    QNetworkAccessManager nm;
    QFile targetFile("ZsyncCorePrivate.dat");
    if(!targetFile.open(QIODevice::WriteOnly)){
	    return -1;
    }
    ZsyncRemoteControlFileParserPrivate cfp(&nm);
    ZsyncCorePrivate *zsync = nullptr;
    cfp.setControlFileUrl(QUrl(QString(argv[1])));
   
    QObject::connect(&cfp , &ZsyncRemoteControlFileParserPrivate::receiveControlFile , [&](){
	zsync = new ZsyncCorePrivate(cfp.getTargetFileBlockSize() , 0 , cfp.getTargetFileBlocksCount(), cfp.getWeakCheckSumBytes(),
				     cfp.getStrongCheckSumBytes(), cfp.getConsecutiveMatchNeeded() ,
				     cfp.getCheckSumBlocksBuffer() , &targetFile);

	zsync->start(cfp.getTargetFileName());
	targetFile.resize(cfp.getTargetFileLength());
        targetFile.close();
	app.quit();	
    });
    cfp.getControlFile();
    return app.exec();
}
