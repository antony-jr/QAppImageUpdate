#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncCore_p.hpp>
#include <QtConcurrent>
#include <QFuture>

using namespace AppImageUpdaterBridge;

static QList<ZsyncCorePrivate::JobInformation> getJobs(size_t blockSize,
						       size_t blocks,
						       qint32 wcksumb,
						       qint32 scksumb,
						       qint32 sm,
						       QBuffer *buffer,
						       QFile *targetFile)
{
	QList<ZsyncCorePrivate::JobInformation> result;
	QBuffer *partition = nullptr;
	qDebug() << "Total Blocks:: " << blocks;
	int firstThreadBlocksToDo = 0,
	    otherThreadsBlocksToDo = 0;
	auto mod = (int)blocks % QThread::idealThreadCount();
	if(mod > 0){
		firstThreadBlocksToDo = (int)(((int)blocks - mod) / QThread::idealThreadCount())
					+ mod;
		otherThreadsBlocksToDo = firstThreadBlocksToDo - mod;
	}else{ // mod == 0
		firstThreadBlocksToDo = (int)((int)blocks / QThread::idealThreadCount());
		otherThreadsBlocksToDo = firstThreadBlocksToDo;
	}

	if(!buffer->isOpen()){
		buffer->open(QIODevice::ReadOnly);
	}

	partition = new QBuffer;
	partition->open(QIODevice::WriteOnly);
	partition->write(buffer->read( firstThreadBlocksToDo * (wcksumb + scksumb)));
	partition->close();

	{
	ZsyncCorePrivate::JobInformation info(blockSize, 0 , firstThreadBlocksToDo , wcksumb, scksumb, sm, partition,targetFile);
	result.append(info);
	}

	if(otherThreadsBlocksToDo){
	int threadCount = 2;
	while(threadCount <= QThread::idealThreadCount()){
	auto fromId = firstThreadBlocksToDo * (threadCount - 1);
	partition = new QBuffer;
	partition->open(QIODevice::WriteOnly);
	partition->write(buffer->read(otherThreadsBlocksToDo * (wcksumb + scksumb)));
	qDebug() << "thread(" << threadCount << ") buffer->atEnd() :: " << buffer->atEnd();
	partition->close();
	{
	ZsyncCorePrivate::JobInformation info(blockSize, fromId, otherThreadsBlocksToDo , wcksumb, scksumb,
					      sm, partition, targetFile);
	result.append(info);
	}
	++threadCount;
	}
	}

	buffer->close();
	delete buffer;
	return result;
}

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
    cfp.setControlFileUrl(QUrl(QString(argv[1])));
   
    QObject::connect(&cfp , &ZsyncRemoteControlFileParserPrivate::receiveControlFile , [&](){
	auto jobs = getJobs(cfp.getTargetFileBlockSize() ,
			    cfp.getTargetFileBlocksCount(), cfp.getWeakCheckSumBytes(),
			    cfp.getStrongCheckSumBytes(), cfp.getConsecutiveMatchNeeded() ,
			    cfp.getCheckSumBlocksBuffer() , &targetFile);


	qDebug() << "Assigned to " << jobs.size() << " Threads.";

	const QString seedFile = cfp.getTargetFileName();
	auto future = QtConcurrent::map(jobs , [&](ZsyncCorePrivate::JobInformation info){
	ZsyncCorePrivate zsync(info);
	auto r = zsync.start(seedFile);
	qDebug() << "GOT BLOCKS:: " << r->first;
	return;
	});

	if(future.isStarted()){
		qDebug() << "Started Thread Pool.";
	}

	future.waitForFinished();
	targetFile.waitForBytesWritten(10 * 1000);
	targetFile.resize(cfp.getTargetFileLength());
        targetFile.close();
	app.quit();	
    });
    cfp.getControlFile();
    return app.exec();
}
