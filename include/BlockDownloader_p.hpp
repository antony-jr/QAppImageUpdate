#ifndef APPIMAGE_UPDATER_BRIDGE_BLOCK_DOWNLOADER_PRIVATE_HPP_INCLUDED
#define APPIMAGE_UPDATER_BRIDGE_BLOCK_DOWNLOADER_PRIVATE_HPP_INCLUDED
#include <BlockReply_p.hpp>
#include <ZsyncWriter_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <QNetworkAccessManager>

namespace AppImageUpdaterBridge {
class BlockDownloaderPrivate : public QObject {
	Q_OBJECT
public:
	explicit BlockDownloaderPrivate(ZsyncRemoteControlFileParserPrivate *parser , ZsyncWriterPrivate *writer , QNetworkAccessManager *nm)
		: QObject(),
		  _pParser(parser),
		  _pWriter(writer),
		  _pManager(nm)
	{
		connect(_pWriter , &ZsyncWriterPrivate::finished , this , &BlockDownloaderPrivate::initDownloader);
		connect(_pWriter , &ZsyncWriterPrivate::blockRange , this , &BlockDownloaderPrivate::handleBlockRange);
		return;
	}

	~BlockDownloaderPrivate()
	{
		return;
	}

public Q_SLOTS:
	void cancel(void)
	{
		emit cancelAllReply();
		return;
	}

private Q_SLOTS:
	void initDownloader(bool doStart)
	{
		if(!doStart)
			return;


		connect(_pParser , &ZsyncRemoteControlFileParserPrivate::targetFileUrl ,
			this , &BlockDownloaderPrivate::handleTargetFileUrl);
		_pParser->getTargetFileUrl();
		return;
	}
	
	void handleTargetFileUrl(QUrl url)
	{
	QNetworkRequest currentRequest;
	currentRequest.setUrl(url);
	currentRequest.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    	currentRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(0) + "-";
        rangeHeaderValue += QByteArray::number(1); // Just get the first 1 Byte data.
	currentRequest.setRawHeader("Range", rangeHeaderValue);	
	auto reply = _pManager->get(currentRequest);
	connect(reply , &QNetworkReply::downloadProgress , this , &BlockDownloaderPrivate::checkHead);
	connect(reply , SIGNAL(error(QNetworkReply::NetworkError)) , this , SLOT(handleCheckHeadError(QNetworkReply::NetworkError)));
	return;
	}

	void handleBlockRange(qint32 fromRange , qint32 toRange)
	{
	QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(fromRange) + "-";
        rangeHeaderValue += QByteArray::number(toRange);
	QNetworkRequest currentRequest;
	currentRequest.setUrl(_uTargetFileUrl);
	currentRequest.setRawHeader("Range", rangeHeaderValue);
	currentRequest.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    	currentRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	auto blockReply = new BlockReplyPrivate(_pWriter , _pManager->get(currentRequest) , fromRange , toRange);
	connect(this , &BlockDownloaderPrivate::cancelAllReply , blockReply , &BlockReplyPrivate::cancel);
	connect(blockReply , &BlockReplyPrivate::error , this , &BlockDownloaderPrivate::error , Qt::DirectConnection);
	return;
	}

	void handleCheckHeadError(QNetworkReply::NetworkError ecode)
	{
		if(ecode == QNetworkReply::OperationCanceledError)
			return;

		emit error(ecode);
		return;
	}

	void checkHead(qint64 br , qint64 bt)
	{
		Q_UNUSED(br);
		Q_UNUSED(bt);

		auto reply = (QNetworkReply*)QObject::sender();
		disconnect(reply , &QNetworkReply::downloadProgress , this , &BlockDownloaderPrivate::checkHead);
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

Q_SIGNALS:
	void error(QNetworkReply::NetworkError);
	void cancelAllReply(void);

private:
	QUrl _uTargetFileUrl;
	ZsyncRemoteControlFileParserPrivate *_pParser = nullptr;
	ZsyncWriterPrivate *_pWriter = nullptr;
	QNetworkAccessManager *_pManager = nullptr;
};
};
#endif // APPIMAGE_UPDATER_BRIDGE_BLOCK_DOWNLOADER_PRIVATE_HPP_INCLUDED
