#include "rangedownloader.hpp"
#include "rangedownloader_p.hpp"
#include "helpers_p.hpp"

RangeDownloader::RangeDownloader(QNetworkAccessManager *manager, QObject *parent) 
		: QObject(parent) {
		m_Private = QSharedPointer<RangeDownloaderPrivate>(new RangeDownloaderPrivate(manager));
		auto obj = m_Private.data();

		connect(obj, &RangeDownloaderPrivate::started, 
			 this, &RangeDownloader::started,
			 Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::canceled, 
			 this, &RangeDownloader::canceled,
			 Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::finished, 
			this, &RangeDownloader::finished,
			 Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::error, 
			 this, &RangeDownloader::error,
			  Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::data, 
			 this, &RangeDownloader::data,
			 Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::rangeData, 
			 this, &RangeDownloader::rangeData,
			 Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::progress,
			 this, &RangeDownloader::progress,
			 Qt::DirectConnection);
}


void RangeDownloader::setBlockSize(qint32 blockSize) {
    getMethod(m_Private.data(), "setBlockSize(qint32)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection,
                    Q_ARG(qint32, blockSize));

}



void RangeDownloader::setTargetFileUrl(const QUrl &url) {
    getMethod(m_Private.data(), "setTargetFileUrl(const QUrl&)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection,
                    Q_ARG(QUrl,url));

}

void RangeDownloader::setTargetFileLength(qint32 n) {
    getMethod(m_Private.data(), "setTargetFileLength(qint32)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection,
                    Q_ARG(qint32,n));
}	



void RangeDownloader::setFullDownload(bool choice){
    getMethod(m_Private.data(), "setFullDownload(bool)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection,
                    Q_ARG(bool,choice));
}

void RangeDownloader::appendRange(qint32 from, qint32 to) {
    getMethod(m_Private.data(), "appendRange(qint32,qint32)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection,
                    Q_ARG(qint32,from), Q_ARG(qint32,to));
}	

void RangeDownloader::start() {
    getMethod(m_Private.data(), "start(void)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection);
}

void RangeDownloader::cancel() {
    getMethod(m_Private.data(), "cancel(void)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection);
}

