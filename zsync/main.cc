#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncCoreWorker_p.hpp>

using namespace AppImageUpdaterBridgePrivate;

int main(int ac, char **av)
{
    if(ac == 1) {
        qInfo().noquote() << "Usage: " << av[0] << " [zsync meta file]";
        return 0;
    }
    QCoreApplication app(ac , av);
    ZsyncCoreWorker *z;
    ZsyncRemoteControlFileParserPrivate p;//(QUrl("http://127.0.0.1:8080/zsync.zsync"));
    p.setControlFileUrl(QUrl("http://127.0.0.1:8080/zsync.zsync"));

    QObject::connect(&p , &ZsyncRemoteControlFileParserPrivate::error , [&](short errorCode){
		    qCritical() << "FATAL:: " << errorCode;
		    return;
    });

    QObject::connect(&p , &ZsyncRemoteControlFileParserPrivate::receiveControlFile ,
		     [&](size_t blocks , size_t blocksize , qint32 rsum_bytes , 
			 qint32 checksum_bytes , qint32 seq_matches , size_t length)
    {
    	qInfo() << "Info:: Blocks = " << blocks << " , BlockSize = " << blocksize << " , rsum_bytes = " << rsum_bytes
		<< " , checksum_bytes = " << checksum_bytes << " , seq_matches = " << seq_matches
		<< " , filesize = " << length;

    	z = new ZsyncCoreWorker(blocks , blocksize , (int)rsum_bytes , (int)checksum_bytes , (int)seq_matches ,length );
	QObject::connect(&p , &ZsyncRemoteControlFileParserPrivate::receiveTargetFileBlocks ,
			[&](zs_blockid id , rsum r , void *checksum){
			qInfo() << "Blockid(" << id << "):: " << QByteArray((char*)checksum).toHex();
			z->add_target_block(id , r , checksum);
	});

	QObject::connect(&p , &ZsyncRemoteControlFileParserPrivate::endOfTargetFileBlocks,
			[&](){
			QFile file(p.getTargetFileName());
			file.open(QIODevice::ReadOnly);
			z->submit_source_file(&file);
			file.close();
	});
	p.getTargetFileBlocks();
    });

    p.getControlFile();

    return app.exec();
}
