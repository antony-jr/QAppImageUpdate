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
    QString filename(av[1]);
    ZsyncCoreWorker *z;
    ZsyncRemoteControlFileParserPrivate p;//(QUrl("http://127.0.0.1:8080/zsync.zsync"));
    p.setControlFileUrl(QUrl("http://127.0.0.1:8080/zsync.zsync"));

    QTimer::singleShot(1000 , [&](){
		z = new ZsyncCoreWorker((size_t)(p.getTargetFileBlocksCount()) ,
					(size_t)(p.getTargetFileBlockSize()),
					(int)(p.getWeakCheckSumBytes()),
					(int)(p.getStrongCheckSumBytes()),
					(int)(p.getConsecutiveMatchNeeded()),
					(size_t)(p.getTargetFileLength()));
		p.getTargetFileBlocks();

		QObject::connect(&p , &ZsyncRemoteControlFileParserPrivate::handleTargetFileBlocks,
		[&](zs_blockid id , rsum r , void *checksum){
			z->add_target_block(id , r , checksum);
		});

		QObject::connect(&p , &ZsyncRemoteControlFileParserPrivate::gotAllTargetFileBlocks , [&](){
			QFile file(p.getTargetFileName());
			file.open(QIODevice::ReadOnly);
			z->submit_source_file(&file);
			file.close();
			return;
		});
		
    });

    return app.exec();
}
