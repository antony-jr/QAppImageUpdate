#ifndef BLOCK_DOWNLOADER_HPP_INCLUDED
#define BLOCK_DOWNLOADER_HPP_INCLUDED
#include <ZsyncWriter_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

Q_DECLARE_METATYPE(QByteArray*);

namespace AppImageUpdaterBridge {
class BlockDownloader : public QObject {
	Q_OBJECT
public:
	BlockDownloader(ZsyncWriterPrivate *writer , ZsyncRemoteControlFileParserPrivate *cp , QNetworkAccessManager *nm)
		: QObject(writer),
		  _pDeltaWriter(writer),
		  _pControlFileParser(cp),
		  _pManager(nm)
	{
		qRegisterMetaType<QByteArray*>();
		connect(writer , &ZsyncWriterPrivate::blockRange ,
			this , &BlockDownloader::handleBlockRange);
		connect(writer , &ZsyncWriterPrivate::finished ,
			this , &BlockDownloader::handleFinished);
		connect(cp, &ZsyncRemoteControlFileParserPrivate::targetFileUrl ,
			this , &BlockDownloader::handleTargetFileUrl);
		return;
	}

	~BlockDownloader()
	{
		return;
	}
public Q_SLOTS:
	void start(void){
		/*
		connect(_pmanager , &qnetworkaccessmanager::finished,
			this , &blockdownloader::handlerequestfinished);
		*/
		_pControlFileParser->getTargetFileUrl();
		return;
	}

private Q_SLOTS:
	void handleTargetFileUrl(QUrl url)
	{
	QNetworkRequest currentRequest;
	currentRequest.setUrl(url);
	currentRequest.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    	currentRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	auto reply = _pManager->get(currentRequest);
	connect(reply , &QNetworkReply::downloadProgress , this , &BlockDownloader::handleGetHeadReq);
	return;
	}

	void handleGetHeadReq(qint64 br , qint64 bt)
	{
	QNetworkReply *reply = (QNetworkReply*)QObject::sender();
	disconnect(reply , &QNetworkReply::downloadProgress , this , &BlockDownloader::handleGetHeadReq);
	_uTargetFileUrl = QUrl(reply->url());
	reply->abort();
	reply->deleteLater();
	connect(_pManager , &QNetworkAccessManager::finished,
		this , &BlockDownloader::handleRequestFinished);
	qDebug() << "Using url:: " << _uTargetFileUrl;
	_pDeltaWriter->getBlockRanges();
	return;
	}

	void handleFinished(bool targetFileConstructed)
	{
		disconnect(_pManager , &QNetworkAccessManager::finished,
			this , &BlockDownloader::handleRequestFinished);
		if(targetFileConstructed){
			emit finished();
		}
		return;
	}

	void handleBlockRange(qint32 fromRange, qint32 toRange)
	{
	qDebug() << "Downloading (" << fromRange << " , " << toRange << " ).";
	QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(fromRange) + "-";
        rangeHeaderValue += QByteArray::number(toRange);
	QNetworkRequest currentRequest;
	currentRequest.setUrl(_uTargetFileUrl);
	currentRequest.setRawHeader("Range", rangeHeaderValue);
	currentRequest.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    	currentRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	_pManager->get(currentRequest);
	return;
	}

	void handleRequestFinished(QNetworkReply *reply)
	{
		QByteArray *data = new QByteArray(reply->readAll());
		QNetworkRequest req = reply->request();
		QByteArray rangeHeader = req.rawHeader("Range");

		rangeHeader = rangeHeader.replace("bytes=" , "");
		auto from = ((rangeHeader.split('-')).at(0)).toInt();
		auto to   = ((rangeHeader.split('-')).at(1)).toInt();
		reply->deleteLater();
		
		qDebug() << "Download Finished (" << from << " , " << to << " ).";

		auto metaObject = _pDeltaWriter->metaObject();
		metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("writeBlockRanges(qint32 , qint32 , QByteArray*)")))
		    .invoke(_pDeltaWriter , Qt::QueuedConnection , Q_ARG(qint32 , from) , Q_ARG(qint32 , to) , Q_ARG(QByteArray* , data));
		return;
	}
Q_SIGNALS:
	void finished(void);
private:
	QUrl _uTargetFileUrl;
	QNetworkAccessManager *_pManager = nullptr;
	ZsyncWriterPrivate *_pDeltaWriter = nullptr;
	ZsyncRemoteControlFileParserPrivate *_pControlFileParser = nullptr;
};
}
#endif // BLOCK_DOWNLOADER_HPP_INCLUDED
