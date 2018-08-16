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
	connect(reply , &QNetworkReply::finished ,
		this , &ZsyncBlockRangeReplyPrivate::handleFinished);
	connect(reply , SIGNAL(error(QNetworkReply::NetworkError)) , 
		this , SLOT(handleError(QNetworkReply::NetworkError)));
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
	QByteArray *rawData = new QByteArray;

	while(!reply->atEnd()){
		/* Use standard size of 4096 bytes. */
		rawData->append(reply->read(4096));
	}
		
	disconnect(this , &ZsyncBlockRangeReplyPrivate::cancelReply , reply , &QNetworkReply::abort);
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

