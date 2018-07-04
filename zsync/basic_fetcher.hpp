#pragma once
#include <QtCore>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <ZsyncCoreWorker_p.hpp>

namespace AppImageUpdaterBridge_p {


class basic_fetcher : public QObject {
	Q_OBJECT
public:
	basic_fetcher(ZsyncCoreWorker *worker , const QUrl &url , size_t blocksize)
		: QObject(worker),
		  url(url),
		  blocksize(blocksize)
	{
		w = worker;
		ranges = w->needed_block_ranges(0 , 0x7fffffff);
		qInfo() << "to do:: " << ranges.size() << " blocks.";
		qInfo() << "blocks:: "<< ranges;
		connect(&manager, &QNetworkAccessManager::finished , this , &basic_fetcher::handleRangeResponse);
		return;
	}

	void start_fetch(void)
	{
	if(ranges.size() > currentIndex){
	QByteArray rangeHeaderValue = "bytes=" + QByteArray::number((quint32)(ranges.at(currentIndex).first*blocksize)) + "-";
        rangeHeaderValue += QByteArray::number((quint32)(((ranges.at(currentIndex).second) *blocksize)));
	QNetworkRequest currentRequest;
	currentRequest.setUrl(url);
	currentRequest.setRawHeader("Range", rangeHeaderValue);
	manager.get(currentRequest);
	}else{
		emit(finishedAll());
	}
	return;
	}

	~basic_fetcher()
	{
		return;
	}

Q_SIGNALS:
	void finishedAll(void);

private Q_SLOTS:
	void handleRangeResponse(QNetworkReply *reply)
	{
	qInfo() << "fetched block:: " << ranges.at(currentIndex).first << " - " << ranges.at(currentIndex).second;

	QByteArray data = reply->readAll();
	unsigned char *d = (unsigned char*)calloc( 1 , ((ranges.at(currentIndex).second - ranges.at(currentIndex).first)*2048) * sizeof(unsigned char));

	memmove(d , data.constData() , sizeof(unsigned char)*data.size());

	if(-1 == w->submit_blocks((unsigned char*)d , ranges.at(currentIndex).first , ranges.at(currentIndex).second)){
		qCritical() << "FATAL:: ("<< ranges.at(currentIndex).first << " - " << ranges.at(currentIndex).second
			   << ") :: Strong Checksum Mismatch.";
	}

	currentIndex++;
	start_fetch();
	return;
	}
private:
	int currentIndex = 0;
	QUrl url;
	size_t blocksize;
	QNetworkAccessManager manager;
	ZsyncCoreWorker *w = nullptr;
	QVector<QPair<zs_blockid , zs_blockid>> ranges;
};
}
