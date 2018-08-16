#include <ZsyncBlockRangeDownloader_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncWriter_p.hpp>

using namespace AppImageUpdaterBridge;

explicit ZsyncBlockRangeDownloaderPrivate::ZsyncBlockRangeDownloaderPrivate(ZsyncRemoteControlFileParserPrivate *parser ,
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
	connect(_pParser , &ZsyncRemoteControlFileParserPrivate::targetFileUrl ,
		this , &ZsyncBlockRangeDownloaderPrivate::handleTargetFileUrl);
	_pParser->getTargetFileUrl();
	return;
}
	
void ZsyncBlockRangeDownloaderPrivate::handleTargetFileUrl(QUrl url)
{
	QNetworkRequest request;
	QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(0) + "-";
        rangeHeaderValue += QByteArray::number(1); // Just get the first 1 Byte data.
	request.setUrl(url);
	request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	request.setRawHeader("Range", rangeHeaderValue);	
	auto reply = _pManager->get(request);
	connect(reply , &QNetworkReply::downloadProgress ,
		this , &ZsyncBlockRangeDownloaderPrivate::checkHead);
	connect(reply , SIGNAL(error(QNetworkReply::NetworkError)) ,
		this , SLOT(handleCheckHeadError(QNetworkReply::NetworkError)));
	return;
}

void ZsyncBlockRangeDownloaderPrivate::handleBlockRange(qint32 fromRange , qint32 toRange)
{
	QNetworkRequest request;
	QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(fromRange) + "-";
        rangeHeaderValue += QByteArray::number(toRange);
	request.setUrl(_uTargetFileUrl);
	request.setRawHeader("Range", rangeHeaderValue);
	request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	
	++_nBlockReply;
	
	auto blockReply = new BlockReplyPrivate(_pWriter , _pManager->get(request) , fromRange , toRange);
	connect(this , &ZsyncBlockRangeDownloaderPrivate::cancelAllReply ,
		blockReply , &BlockReplyPrivate::cancel);
	connect(blockReply , &BlockReplyPrivate::finished ,
		this , &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyFinished ,
		Qt::QueuedConnection);
	connect(blockReply , &BlockReplyPrivate::canceled ,
		this , &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyCancel ,
		Qt::QueuedConnection);
	connect(blockReply , &BlockReplyPrivate::error ,
		this , &ZsyncBlockRangeDownloaderPrivate::error ,
		Qt::DirectConnection);
	return;
}

void ZsyncBlockRangeDownloaderPrivate::handleBlockReplyFinished(void)
{
	auto blockReply = (BlockReplyPrivate*)QObject::sender();
	blockReply->deleteLater();
	
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
	auto blockReply = (BlockReplyPrivate*)QObject::sender();
	blockReply->deleteLater();
	
	--_nBlockReply;
		
	if(_nBlockReply <= 0){
		_bCancelRequested = false;
		emit canceled();
	}
	return;
}

void ZsyncBlockRangeDownloaderPrivate::handleCheckHeadError(QNetworkReply::NetworkError ecode)
{
	if(ecode == QNetworkReply::OperationCanceledError){ return; }
	emit error(ecode);
	return;
}

void ZsyncBlockRangeDownloaderPrivate::checkHead(qint64 br , qint64 bt)
{
	Q_UNUSED(br);
	Q_UNUSED(bt);

	auto reply = (QNetworkReply*)QObject::sender();
	disconnect(reply , &QNetworkReply::downloadProgress , this , &ZsyncBlockRangeDownloaderPrivate::checkHead);
	auto replyCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		
	reply->abort();
	if(replyCode != 206)
	{
		return;
	}

	_uTargetFileUrl = reply->url();
	auto metaObject = _pWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getBlockRanges(void)")))
		   .invoke(_pWriter , Qt::QueuedConnection);
	return;
}
