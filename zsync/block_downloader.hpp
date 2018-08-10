#ifndef BLOCK_DOWNLOADER_HPP_INCLUDED
#define BLOCK_DOWNLOADER_HPP_INCLUDED
#include <AppImageDeltaWriter>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace AppImageUpdaterBridge {
class BlockDownloader : public QObject {
	Q_OBJECT
public:
	BlockDownloader(AppImageDeltaWriter *writer)
		: QObject(writer),
		  _pDeltaWriter(writer)
	{
		connect(writer , &AppImageDeltaWriter::blockRange ,
			this , &BlockDownloader::handleBlockRange);
		connect(writer , &AppImageDeltaWriter::finished ,
			this , &BlockDownloader::handleFinished);
		connect(writer, &AppImageDeltaWriter::targetFileUrl ,
			this , &BlockDownloader::handleTargetFileUrl);
		return;
	}

	~BlockDownloader()
	{
		return;
	}
public Q_SLOTS:
	void start(void){
		auto networkManager = (QNetworkAccessManager*)_pDeltaWriter->sharedNetworkAccessManager();
		connect(networkManager , &QNetworkAccessManager::finished,
			this , &BlockDownloader::handleRequestFinished);
		_pDeltaWriter->getTargetFileUrl();
		return;
	}

private Q_SLOTS:
	void handleTargetFileUrl(QUrl url)
	{
		_uTargetFileUrl = url;
		_pDeltaWriter->getBlockRanges();
		return;
	}
	void handleFinished(bool targetFileConstructed)
	{
		auto networkManager = (QNetworkAccessManager*)_pDeltaWriter->sharedNetworkAccessManager();
		disconnect(networkManager , &QNetworkAccessManager::finished,
			this , &BlockDownloader::handleRequestFinished);
		if(targetFileConstructed){
			emit finished();
		}
		return;
	}

	void handleBlockRange(QPair<qint32 , qint32> range)
	{
	QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(range.first) + "-";
        rangeHeaderValue += QByteArray::number(range.second);
	QNetworkRequest currentRequest;
	currentRequest.setUrl(_uTargetFileUrl);
	currentRequest.setRawHeader("Range", rangeHeaderValue);
	currentRequest.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    	currentRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	_pDeltaWriter->sharedNetworkAccessManager()->get(currentRequest);
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

		QPair<qint32 , qint32> range = qMakePair( from , to );

		_pDeltaWriter->writeBlockRanges(range , data);
		return;
	}
Q_SIGNALS:
	void finished(void);
private:
	QUrl _uTargetFileUrl;
	AppImageDeltaWriter *_pDeltaWriter = nullptr;
};
}
#endif // BLOCK_DOWNLOADER_HPP_INCLUDED
