#include <ZsyncBlockRangeDownloader_p.hpp>
#include <ZsyncBlockRangeReply_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncWriter_p.hpp>

using namespace AppImageUpdaterBridge;

ZsyncBlockRangeDownloaderPrivate::ZsyncBlockRangeDownloaderPrivate(ZsyncRemoteControlFileParserPrivate *parser ,
					  				    ZsyncWriterPrivate *writer ,
					  				    QNetworkAccessManager *nm)
		 : QObject(),
		  _pParser(parser),
		  _pWriter(writer),
		  _pManager(nm)
{
	connect(_pWriter , &ZsyncWriterPrivate::finished , 
		this , &ZsyncBlockRangeDownloaderPrivate::initDownloader);
	connect(_pWriter , &ZsyncWriterPrivate::blockRange , 
		this , &ZsyncBlockRangeDownloaderPrivate::handleBlockRange ,Qt::QueuedConnection);
	return;
}

ZsyncBlockRangeDownloaderPrivate::~ZsyncBlockRangeDownloaderPrivate()
{
	return;
}

void ZsyncBlockRangeDownloaderPrivate::cancel(void)
{
	_bCancelRequested = true;
	emit cancelAllReply();
	return;
}

void ZsyncBlockRangeDownloaderPrivate::initDownloader(bool doStart)
{
	if(!doStart){ return; }

	auto writerMetaObject = _pWriter->metaObject();
	
	_nBytesReceived = _pWriter->getBytesWritten(); // set atomic integer.
	_nBytesTotal = _pParser->getTargetFileLength();
	_uTargetFileUrl = _pParser->getTargetFileUrl();
	
	/*
	 * Start the download , if the host cannot accept range requests then
	 * blockRange signal will return with both 'from' and 'to' ranges 0.
	*/
	emit started();
	writerMetaObject->method(writerMetaObject->indexOfMethod(QMetaObject::normalizedSignature("getBlockRanges(void)")))
		   .invoke(_pWriter , Qt::QueuedConnection);
	return;
}

void ZsyncBlockRangeDownloaderPrivate::handleBlockRange(qint32 fromRange , qint32 toRange)
{
	QNetworkRequest request;
	request.setUrl(_uTargetFileUrl);
	if(fromRange || toRange){
	QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(fromRange) + "-";
        rangeHeaderValue += QByteArray::number(toRange);
	request.setRawHeader("Range", rangeHeaderValue);
	}
	request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	
	++_nBlockReply;

	auto blockReply = new ZsyncBlockRangeReplyPrivate(_pWriter , _pManager->get(request) , fromRange , toRange);
	connect(this , &ZsyncBlockRangeDownloaderPrivate::cancelAllReply ,
		blockReply , &ZsyncBlockRangeReplyPrivate::cancel);
	connect(blockReply , &ZsyncBlockRangeReplyPrivate::canceled ,
		this , &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyCancel ,
		Qt::QueuedConnection);
	if(!(fromRange || toRange)){
	connect(blockReply , &ZsyncBlockRangeReplyPrivate::seqProgress , 
		this , &ZsyncBlockRangeDownloaderPrivate::progress , Qt::DirectConnection);
	connect(blockReply , &ZsyncBlockRangeReplyPrivate::finished ,
		this , &ZsyncBlockRangeDownloaderPrivate::finished ,
		Qt::DirectConnection);
	connect(blockReply , &ZsyncBlockRangeReplyPrivate::canceled ,
		this , &ZsyncBlockRangeDownloaderPrivate::canceled ,
		Qt::DirectConnection);
	}else{
	connect(blockReply , &ZsyncBlockRangeReplyPrivate::progress , 
		this , &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyProgress ,
		Qt::QueuedConnection);
	connect(blockReply , &ZsyncBlockRangeReplyPrivate::finished ,
		this , &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyFinished ,
		Qt::QueuedConnection);
	connect(blockReply , &ZsyncBlockRangeReplyPrivate::canceled ,
		this , &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyCancel ,
		Qt::QueuedConnection);
	}
	connect(blockReply , &ZsyncBlockRangeReplyPrivate::error ,
		this , &ZsyncBlockRangeDownloaderPrivate::error ,
		Qt::DirectConnection);
	return;
}

void ZsyncBlockRangeDownloaderPrivate::handleBlockReplyProgress(qint64 bytesReceived , double speed , QString units)
{
	_nBytesReceived += bytesReceived;
	int nPercentage = static_cast<int>(
            		(static_cast<float>
             		 (_nBytesReceived.load()) * 100.0
           		) / static_cast<float>
            		 (_nBytesTotal)
        	      );
	emit progress(nPercentage , _nBytesReceived.load() , _nBytesTotal , speed , units);
	return;
}

void ZsyncBlockRangeDownloaderPrivate::handleBlockReplyFinished(void)
{
//	auto blockReply = (ZsyncBlockRangeReplyPrivate*)QObject::sender();
//	blockReply->deleteLater();
	
	--_nBlockReply;
		
	if(_nBlockReply <= 0){
		if(_bCancelRequested == true)
		{
			_bCancelRequested = false;
			emit canceled();
		}else{
			emit finished();
		}
	}
	return;
}

void ZsyncBlockRangeDownloaderPrivate::handleBlockReplyCancel(void)
{
	auto blockReply = (ZsyncBlockRangeReplyPrivate*)QObject::sender();
	blockReply->deleteLater();
	
	--_nBlockReply;
		
	if(_nBlockReply <= 0){
		_bCancelRequested = false;
		emit canceled();
	}
	return;
}
