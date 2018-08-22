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
	_pRawData.reset(new QByteArray);
	connect(reply , SIGNAL(error(QNetworkReply::NetworkError)) , 
		this , SLOT(handleError(QNetworkReply::NetworkError)));
	if(!(_nRangeFrom || _nRangeTo)){
	connect(reply , &QNetworkReply::finished ,
		this , &ZsyncBlockRangeReplyPrivate::finished);
	connect(reply , &QNetworkReply::downloadProgress ,
		this , &ZsyncBlockRangeReplyPrivate::handleSeqProgress);
	connect(this , &ZsyncBlockRangeReplyPrivate::sendData , 
		deltaWriter , &ZsyncWriterPrivate::rawSeqWrite , Qt::QueuedConnection);
	}else{
	connect(reply , &QNetworkReply::finished ,
		this , &ZsyncBlockRangeReplyPrivate::handleFinished);
	connect(reply , &QNetworkReply::downloadProgress ,
		this , &ZsyncBlockRangeReplyPrivate::handleProgress);
	connect(this ,  &ZsyncBlockRangeReplyPrivate::sendBlockDataToWriter ,
		deltaWriter , &ZsyncWriterPrivate::writeBlockRanges , Qt::QueuedConnection);
	}
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
	_pRawData->append(reply->readAll());
	emit sendBlockDataToWriter(_nRangeFrom , _nRangeTo , _pRawData.take());
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

void ZsyncBlockRangeReplyPrivate::handleSeqProgress(qint64 bytesReceived , qint64 bytesTotal)
{
	auto reply = (QNetworkReply*)QObject::sender();
	
	auto data = new QByteArray(reply->readAll());
	emit sendData(data);

	int nPercentage = static_cast<int>(
            		(static_cast<float>
             		 (bytesReceived) * 100.0
           		) / static_cast<float>
            		 (bytesTotal)
        	      );

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
	emit seqProgress(nPercentage , bytesReceived , bytesTotal , nSpeed , sUnit);
	return;
}

void ZsyncBlockRangeReplyPrivate::handleProgress(qint64 bytesReceived , qint64 bytesTotal)
{
	Q_UNUSED(bytesTotal);
	auto reply = (QNetworkReply*)QObject::sender();
	qint64 nowReceived = bytesReceived - _nPreviousBytesReceived;
	_nPreviousBytesReceived = bytesReceived;

	_pRawData->append(reply->readAll());

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

