#include <ZsyncBlockRangeReply_p.hpp>
#include <ZsyncWriter_p.hpp>

using namespace AppImageUpdaterBridge;

ZsyncBlockRangeReplyPrivate::ZsyncBlockRangeReplyPrivate(ZsyncWriterPrivate *deltaWriter ,
							 QNetworkReply *reply ,
							 qint32 rangeFrom ,
							 qint32 rangeTo)
		 : QObject(reply),
		  _nRangeFrom(rangeFrom),
		  _nRangeTo(rangeTo)
{
	downloadSpeed.start();
	connect(reply , &QNetworkReply::finished ,
		this , &ZsyncBlockRangeReplyPrivate::handleFinished);
	connect(reply , SIGNAL(error(QNetworkReply::NetworkError)) , 
		this , SLOT(handleError(QNetworkReply::NetworkError)));
	connect(reply , &QNetworkReply::downloadProgress ,
		this , &ZsyncBlockRangeReplyPrivate::handleProgress);
	connect(this ,  &ZsyncBlockRangeReplyPrivate::sendBlockDataToWriter ,
		deltaWriter , &ZsyncWriterPrivate::writeBlockRanges);
	connect(this ,  &ZsyncBlockRangeReplyPrivate::cancelReply ,
		reply , &QNetworkReply::abort);
	return;
}

ZsyncBlockRangeReplyPrivate::~ZsyncBlockRangeReplyPrivate()
{
	return;
}

void ZsyncBlockRangeReplyPrivate::cancel(void)
{
	emit cancelReply();
	return;
}

void ZsyncBlockRangeReplyPrivate::handleFinished(void)
{
	auto reply = (QNetworkReply*)QObject::sender();
	disconnect(this , &ZsyncBlockRangeReplyPrivate::cancelReply , reply , &QNetworkReply::abort);
	QByteArray *rawData = new QByteArray;
	
	while(!reply->atEnd()){
		rawData->append(reply->read(4096));
		QCoreApplication::processEvents();
	}

	emit sendBlockDataToWriter(_nRangeFrom , _nRangeTo , rawData);
	emit finished();
	return;
}

void ZsyncBlockRangeReplyPrivate::handleError(QNetworkReply::NetworkError ecode)
{
	auto reply = (QNetworkReply*)QObject::sender();
	disconnect(reply , &QNetworkReply::finished , this , &ZsyncBlockRangeReplyPrivate::handleFinished);

	if(ecode == QNetworkReply::OperationCanceledError)
	{
		emit canceled();
		return;
	}
	emit error(ecode);
	return;
}

void ZsyncBlockRangeReplyPrivate::handleProgress(qint64 bytesReceived , qint64 bytesTotal)
{
	Q_UNUSED(bytesTotal);
	qint64 nowReceived = bytesReceived - _nPreviousBytesReceived;
	_nPreviousBytesReceived = bytesReceived;
	QString sUnit;
	double nSpeed =  bytesReceived * 1000.0 / downloadSpeed.elapsed();
    	if (nSpeed < 1024) {
        	sUnit = "bytes/sec";
    	} else if (nSpeed < 1024 * 1024) {
        	nSpeed /= 1024;
        	sUnit = "kB/s";
    	} else {
        	nSpeed /= 1024 * 1024;
        	sUnit = "MB/s";
	}
	emit progress(nowReceived , nSpeed , sUnit);
	return;
}

